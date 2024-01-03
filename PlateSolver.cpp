// Standard math functions
#define _USE_MATH_DEFINES // Declares maths constants
#include <math.h>         //Main math functions
#include <numeric>        //Range based numeric operations
#include <cstdint>        //Provides fixed width integer types

// Type definitions
#include <vector> //Vector container
#include <map>    //Map container

// Utility libraries
#include <cstdlib> //General purpose utilities
#include <chrono>  //Time based utilities
#include <tuple>   //Tuple class template

// Iterators and Algorithms
#include <iterator>  //Range based iterators
#include <algorithm> //Range based algorithms

// Input and Output Libraries
#include <fstream>  //Class templates for streams
#include <iostream> //Stream objects

// Image handling
#include <opencv2/opencv.hpp>

// Further math functions
#include <Eigen/Dense>                             //Type definitions for Eigen objects/classes
#include <unsupported/Eigen/NonLinearOptimization> //C++ implementations of LAPACK (Linear Algebra Package)
#include <unsupported/Eigen/NumericalDiff>         //Approximate numerical differentaition for functors.

// To profile use: perf profile [command] [args]
// To view generated profile, use: perf report

/*    Define key variables    */

const int CURRENT_YEAR = 2023;

int maximum_star_area = 800;
int minimum_star_area = 2;

long double min_star_angle = 8.73e-4;
float max_fov = .349065;
// float max_fov = .1745329;
// float max_fov = .5235987755982988;
const uint32_t KNUTHS_CONSTANT = 2654435769; // 2^32 * (1 - sqrt(5)) / 2 --> https://gist.github.com/badboy/6267743

int img_width = 0;
int img_height = 0;

std::vector<long double> global_match_edges; // for the fov_calculation
std::vector<std::tuple<int, long double, long double>> global_polygon_centroids;

bool save_file = true;
bool save_file_vectors = true;

bool fast = true;

// long double prob_match_threshold = 1e-20;
long double prob_match_threshold = 1e-11;

bool full_output = false; // if true, prints a list of all Ra/Dec matches, and their prob.

/*  Struct definitions  */

struct labeled_array
{
    /*
        Used as a return type of the LabelRegion() function, used to label regions in a binary masked image.
    */

    std::vector<std::vector<int>> regions; // 2D vector of integers representing different regions in the image.
    int count;                             // Total number of regions in the image/array.
};

struct combination_generator
{
    /*
        Used to generate combinations of indicies of length size, and in range 1 - n.

        Code modified from : https://stackoverflarrayow.com/questions/9430568/generating-combinations-in-c/9432150#9432150
    */
    combination_generator(int n, int size) : completed(n < 1 || size > n),
                                             generated(0),
                                             n(n), size(size)
    {
        for (int count = 1; count <= size; ++count)
        {
            current_combination.push_back(count);
        }
    }

    bool completed;
    int generated;

    std::vector<int> next()
    {
        std::vector<int> output_combination = current_combination;

        completed = true;

        for (int i = size - 1; i >= 0; --i)
            if (current_combination[i] < n - size + i + 1)
            {
                int j = current_combination[i] + 1;
                while (i <= size - 1)
                {
                    current_combination[i++] = j++;
                }
                completed = false;
                ++generated;
                break;
            }

        return output_combination;
    }

private:
    int n, size;
    std::vector<int> current_combination;
};

inline long double deg_to_rad(long double deg)
{
    return (deg * (M_PI / 180));
}

inline long double rad_to_deg(long double rad)
{
    return (rad * (180 / M_PI)); 
}

std::vector<std::vector<long double>> calculateStarVectors(std::vector<std::tuple<int, long double, long double>> star_centroids, long double fov)
{
    /*
        For each star in an image, a vector (i, j, k) can be calculated that maps the position of the star from the image center.
        i and j represent the 2D postion of the star, whilst k is used to represent distance / relative magnitude.

        To calculate, the focal length of the camera used to take the image is needed.

        Inputs:
            star_centroids [vector of tuples], Each vector entry represents an individual star, in the format: {Area, Sx, Sy}
            fov [long double], Used to calculate a more accurate focal length, improving accuracy of vectors.

        Outputs:
            star_vectors [2D vector of long doubles], Each vector entry contains the {i, j, k} components calculated for the coresponding star in star_centroids.

        Notes:
            https://core.ac.uk/download/pdf/19894138.pdf
            https://photo.stackexchange.com/questions/97213/finding-focal-length-from-image-size-and-fov
            https://www.researchgate.net/publication/228986403_Spacecraft_angular_rate_estimation_algorithms_for_star_tracker-based_attitude_determination
    */

    std::vector<std::vector<long double>> star_vectors;

    long double img_center = static_cast<long double>(img_width) / 2; // Finds the image center in the x axis
    long double focal_length = (img_center) / tanl(fov / 2);          // Calculates an estimate of the focal length using the field of view.

    for (std::tuple<int, long double, long double> star : star_centroids)
    {
        long double x, y, z, xz, yz; // temporary  variables: (x,y,z) vectors + x/z and y/z ratios

        xz = static_cast<long double>(((img_center)-std::get<2>(star)) / focal_length);
        yz = static_cast<long double>(((static_cast<float>(img_height) / 2.0) - std::get<1>(star)) / focal_length);
        z = 1.0 / (sqrtl(powl(xz, 2) + powl(yz, 2) + 1));
        x = xz * z;
        y = yz * z;

        star_vectors.push_back({z, x, y}); // need to be inverted (eg: z,x,y instead of x,y,z to match with Area, Sx, Sy calculations in the database.)
    }

    return star_vectors;
}

inline long double pythagoras3D(std::vector<long double> star1, std::vector<long double> star2)
{
    /*
        Caculates the distance between two points in 3D, using the 3D Pythagorean Theorem:

            w = sqrt((delta_x)^2 + (delta_y)^2 + (delta_z)^2)

        Inputs:
            star1 [vector, long double], Vector for star 1
            star2 [vector, long double], Vector for star 2

        Output:
            w [long double], Distance between star1 and star2
    */

    long double w, x, y, z;
    x = (star1[0] - star2[0]);
    y = (star1[1] - star2[1]);
    z = (star1[2] - star2[2]);
    w = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
    return w;
}

bool inverseSort(long double a, long double b)
{
    return a > b;
}

bool tupleVectorSort(std::tuple<int, float, float> a, std::tuple<int, float, float> b) // sorts an array of tuples based on the first value in the tuple (area in this case)
{
    return (std::get<0>(a) > std::get<0>(b));
}

bool starCatalogSort(std::tuple<float, long double, long double> a, std::tuple<float, long double, long double> b)
{
    return (std::get<0>(a) < std::get<0>(b)); // was >
}

std::vector<std::vector<int>> generateCombinations(int n, int length)
{
    /*
        Generates all combinations of (length) indexes, in the range (n).

        Inputs:
            n [int], Number of indexes to generate combinations for.
            length [int], Number of indexes in each combination.

        Outputs:
            combinations [2D vector, int], All generated combinations.

        Notes:
            Code modified from: https://stackoverflow.com/questions/9430568/generating-combinations-in-c
    */

    std::vector<std::vector<int>> combinations;
    std::vector<bool> tmp_vector(n);
    std::fill(tmp_vector.begin(), tmp_vector.begin() + length, true);

    do
    {
        std::vector<int> tmp_combination;
        for (int i = 0; i < n; ++i)
        {
            if (tmp_vector[i])
            {
                tmp_combination.push_back(i);
            }
        }
        combinations.push_back(tmp_combination);
    } while (std::prev_permutation(tmp_vector.begin(), tmp_vector.end()));

    return combinations;
}

/*    Define minimization Functors using templates and structs   */

template <typename _Scalar, int N_x = Eigen::Dynamic, int N_y = Eigen::Dynamic>
struct Functor
{
    // https://github.com/daviddoria/Examples/blob/master/c%2B%2B/Eigen/LevenbergMarquardt/CurveFitting.cpp
    enum
    {
        InputsAtCompileTime = N_x,
        ValuesAtCompileTime = N_y
    };

    typedef _Scalar Scalar;
    typedef Eigen::Matrix<Scalar, InputsAtCompileTime, 1> InputType;
    typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, 1> ValueType;
    typedef Eigen::Matrix<Scalar, ValuesAtCompileTime, InputsAtCompileTime> JacobianType;

    int m_inputs, m_values;

    Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {} // Default constructor
    Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}     // Parameterized constructor

    int inputs() const
    {
        return m_inputs;
    }

    int values() const
    {
        return m_values;
    }
};

struct calculateFov : Functor<double>
{
    calculateFov(void) : Functor<double>(6, 6) {}

    int operator()(const Eigen::VectorXd &fov, Eigen::VectorXd &out) const
    {
        std::vector<std::vector<long double>> polygon_vectors = calculateStarVectors(global_polygon_centroids, static_cast<long double>(fov(0)));

        std::vector<long double> polygon_edges;
        std::vector<std::vector<int>> pairs = generateCombinations(4, 2);

        for (std::vector<int> pair : pairs)
        {
            polygon_edges.push_back(pythagoras3D(polygon_vectors[pair[0]], polygon_vectors[pair[1]])); 
        }

        std::sort(polygon_edges.begin(), polygon_edges.end(), inverseSort);

        for (int i = 0; i < 6; i++)
        {
            out(i) = static_cast<double>(global_match_edges[i] - polygon_edges[i]);
        }

        return 0;
    }
};

void stringToLongDoubleVector(const std::string &row, std::vector<long double> &output_row)
{
    /*
        Converts a string into a vector of long doubles. Used when reading the star vector catalogue.

        Inputs:
            row [string, reference], The string of data to be converted.

        Output:
            output_row [vector, long doubles, reference], Vector of long doubles.

        Notes:
            Referenced from: https://stackoverflow.com/questions/1120140/how-can-i-read-and-parse-csv-files-in-c
    */

    size_t i = 0;
    int count = 0;

    std::string prefix = ""; // Prefix is used to stop implicit casting.
    std::vector<std::string> values{prefix};

    try
    {
        for (char chr : row)
        {
            if (count == 1)
            {
                count = 0;
                values.push_back(prefix);
                output_row.push_back(std::stold(values[i++]));
                continue;
            }

            if (chr == ' ')
            {
                count++;
                continue;
            }

            values[i].push_back(chr);
        }

        output_row.push_back(std::stold(values[i]));
        output_row.erase(output_row.end());
    }
    catch (const std::exception &e)
    {
        //Unable to read line, so just skip vector.
    }
}

void loadVectorFile(std::string filename, std::vector<std::vector<long double>> &output)
{
    /*
        Loads the star vector catalogue into a 2D vector of long doubles.
        Each entry contains the i/j/k vector that represents the position of the star.

        Input:
            filename [string], The filename of the vector catalogue file.

        Output:
            output [2D vector, long doubles], The 2D vector of star vectors.
    */

    std::ifstream file(filename);

    std::string row;

    while (!file.eof()) // While not the end of file
    {
        std::getline(file, row); // Read the line of data into a string.

        if (file.fail() || file.bad()) // Break if an error occurs.
        {
            break;
        }

        std::vector<long double> ld_values;

        stringToLongDoubleVector(row, ld_values); // Convert the string into a vector of long doubles.

        output.push_back(ld_values);
    }
}

void loadCatalogueFile(std::string filename, std::vector<std::vector<int>> &out)
{
    /*
        Reads the generated star catalogue file into a 2D vector of ints. Each line in the catalogue contains four ints, which represent the indexes of the stars making up each polygon.


        Inputs:
            filename [string], Filename of the .dat file that stores the binary data.

        Outputs:
            out [2D vector of ints, reference], The output vector that the data is saved to.
    */

    std::ifstream input(filename, std::ios::binary);

    std::vector<int> tmp(4); // Preallocate a vector of size 4, as each line is known to have 4 ints.

    uint16_t x; // Each int is 16 bits in size (2 bytes).
    std::streamsize x_size = sizeof(x);

    while (!input.eof())
    {
        tmp = {};

        for (int i = 0; i < 4; i++)
        {
            input.read(reinterpret_cast<char *>(&x), x_size); // Read x_size bits of data, reinterpreting the data as uint16_t instead of char*
            tmp.push_back(static_cast<int>(x));               // Converting from uint16_t to int for compatibility with rest of program.
        }

        out.push_back(tmp);
    }

    out.erase(out.end() - 1); // Remove the end-of-file data.
}

inline long double dotProduct_3D(std::vector<long double> a, std::vector<long double> b)
{
    /*
        Calculates the dot product between two vectors, using the formula:

            a.b = |a||b|cos(theta) = (a1*b1) + (a2*b2) + (a3*b3) + ... + (an*bn)

        This is used to calculate the angle between two vectors, and thus the angular separation between two stars.

        Inputs:
            a [vector, long double], Vector a.
            b [vector, long double], Vector b.

        Outputs:
            [long double], The dot product of the vectors.

        Notes:
            To increase speed, this function is inlined, and is hardcoded to only work for 3D vectors.
    */

    return ((a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]));
}

std::vector<std::vector<int>> GenerateHashCodes(std::vector<long double> vector, long double error, int hash_space_size)
{
    /*
        Converts a vector of long doubles into a (vector of) vector of ints, to be used for hashing.
        Because hashing with long doubles introduces error, a set of all possible hash codes are generated.
        Each element in the vector is individually hashed, forming a vector hash code.

        Inputs:
            vector [vector, long double], The vector to be converted to hash codes.
            error [long double], The maximum error allowed in the hash.
            hash_space_size [int], How big the hash space is, ie. the bin size of the hash space.

        Outputs:
            hash_codes [2D vector, int], The vector of vector of ints, representing the hashes calculated.
    */

    // Declare variables
    std::vector<int> maximum_values, minimum_values;
    std::vector<std::vector<int>> hash_codes;
    int offset = (hash_space_size == 8);

    for (long double element : vector)
    {
        maximum_values.push_back(1 + static_cast<int>(truncl((element + error + offset) * ((hash_space_size == 8) ? 4 : hash_space_size))));
        minimum_values.push_back(static_cast<int>(truncl((element - error + offset) * ((hash_space_size == 8) ? 4 : hash_space_size))));
    }

    for (int a = std::max(0, minimum_values[0]); a < std::min(hash_space_size, maximum_values[0]); a++)
    {
        for (int b = std::max(0, minimum_values[1]); b < std::min(hash_space_size, maximum_values[1]); b++)
        {
            for (int c = std::max(0, minimum_values[2]); c < std::min(hash_space_size, maximum_values[2]); c++)
            {
                if (offset)
                {
                    hash_codes.push_back({a, b, c});
                    continue;
                }
                for (int d = std::max(0, minimum_values[3]); d < std::min(hash_space_size, maximum_values[3]); d++)
                {
                    for (int e = std::max(0, minimum_values[4]); e < std::min(hash_space_size, maximum_values[4]); e++)
                    {
                        hash_codes.push_back({a, b, c, d, e});
                    }
                }
            }
        }
    }
    return hash_codes;
}

inline std::vector<int> starSearchByRadius(std::vector<long double> star_vector, long double search_radius, std::map<std::vector<int>, std::vector<int>> star_map, std::vector<std::vector<long double>> catalog_vectors)
{
    /*
        Returns the indexes of all stars within a certain search radius of the specified star vector.

        Finds all possible hash codes using the given star vector and the search radius, and uses this to extract all stars from the catalogue.
        For each extracted star, a quick check is made to see if it's within the search radius using the dot product.

        Inputs:
            star_vector [vector, long double], The vector of the specified star.
            search_radius [long double], Defines the maximum angle from the specified star and any matches.
            star_map [map, int vector to int vector], The star map, used to access stars by hash code.
            catalog_vectors [2D vector, long double], The catalogue of star vectors, used for checking angles between stars.

        Outputs:
            output_star_ids [vector, int], The indexes of all stars within a certain search radius.
    */

    std::vector<int> output_star_ids;

    for (std::vector<int> hash : GenerateHashCodes(star_vector, search_radius, 8))
    {
        for (int star_id : star_map[hash])
        {
            if (dotProduct_3D(star_vector, catalog_vectors[star_id]) > cosl(search_radius))
            {
                output_star_ids.push_back(star_id);
            }
        }
    }

    return output_star_ids;
}



int knuthsHash(std::vector<int> code, int size)
{
    /*
        Hashing algorithm, based on Knuth's Multiplicative Method, used to generate the indexes for the polygon catalogue.

        Inputs:
            code [vector, int], The hash code space to be converted to index.
            size [int], Size of the catalogue the indexes are generated for.

        Output:
            val [int], The generated index.

        Notes:
            KNUTHS_CONSTANT is equal to: 2^32 * (1 - sqrt(5)) / 2
            More can be read at: https://gist.github.com/badboy/6267743
    */

    uint32_t val = 0;

    for (int i = 0; i < static_cast<int>(code.size()); i++)
    {
        val += static_cast<uint32_t>(code[i] * pow(25, i));
    }

    val *= KNUTHS_CONSTANT;

    return (val % size);
}

std::vector<std::tuple<float, long double, long double>> readData(std::istream &input, std::ofstream &out_stream)
{
    /*
        Loads data from the Hipparcos star catalogue, calculates the current positions of all stars and loads them into a vector of tuples.

        Inputs:
            input [reference, istream], The input file stream.

        Outputs:
            output [vector, tuple], The updated star catalogue data.
    */

    std::vector<std::tuple<float, long double, long double>> output;
    std::string row;
    int missing = 0; // Add to log

    while (!input.eof())
    {
        std::getline(input, row);
        if (input.fail() || input.bad())
        {
            break; 
        }

        /*
        Position = current position + (rate of movement * time)

        Star catalog (Hipparcos Main) calculated at the Epoch J1991.25, and records current position (Ra, Dec) and Proper Motion (pmRa, pmDec), in milliarcseconds (mas).

        To calculate current positions, convert (pmRa, pmDec) from mas to deg (/60/60/1000), multiply by time passed since J1991.25, and add to original position (Ra, Dec)
        */

        long double ra, dec;
        float mag;
        try
        {
            mag = std::stof(row.substr(41, 5)); 

            ra = deg_to_rad(std::stold(row.substr(51, 12)) + ((std::stold(row.substr(87, 8)) / 3600000) * (CURRENT_YEAR - 1991.25)));
            dec = deg_to_rad(std::stold(row.substr(64, 12)) + ((std::stold(row.substr(96, 8)) / 3600000) * (CURRENT_YEAR - 1991.25)));

            output.push_back(std::make_tuple(mag, ra, dec));
        }
        catch (const std::exception &e)
        {
            missing++;
        }
    }

    out_stream << "Incomplete entries: " << missing << std::endl;
    std::sort(output.begin(), output.end(), starCatalogSort);

    return output;
}

void writeBinaryCatalogue(std::vector<std::vector<int>> output, std::string filename, std::ofstream &out_stream)
{
    /*
        Writes the catalogue to a binary file, to speed up the read times.

        Inputs:
            output [2D vector, int], The catalogue to be written.
            filename [std::string], The name of the file.
    */

    std::ofstream out(filename, std::ios::out | std::ios::binary);

    if (!out)
    {
        std::cerr << "Unable to open file: " << filename << ". Exiting." << std::endl; 
        exit(-1);
    }

    size_t a = out.tellp(); // Gets the initial write position
    uint16_t tmp;

    for (std::vector<int> polygon : output)
    {
        for (int val : polygon)
        {
            tmp = static_cast<uint16_t>(val); // Convert to uint16_t (each int has 16 bits.)
            out.write(reinterpret_cast<char *>(&tmp), sizeof(uint16_t)); // Write value to file
        }
    }

    size_t b = out.tellp(); // Gets the final write position
    out.close();

    out_stream << "Filesize (Bytes): " << (b - a) << std::endl;
}

void generateDatabase(std::vector<std::tuple<float, long double, long double>> catalog, std::string vectors_filename, std::string catalogue_filename, std::ofstream &out_stream) 
{
    /*
        Generates a star catalogue for a specific Field of View using data from the Hipparcos Star Catalogue, and outputs both the vectors and catalogue to files.

        Inputs:
            catalog [vector, tuple], Hipparcos Star Catalogue.
            vectors_filename [std::string], Filepath to where the vectors should be saved.
            catalogue_filename [std::string], Filepath to where the catalogue should be saved.
            out_stream [std::ofstream, reference], Log file output stream.
    */

    /*    Generate all star vectors    */

    // See: http://fmwriters.com/Visionback/Issue14/wbputtingstars.htm
    // See: file:///home/fergus/Downloads/Spacecraft_angular_rate_estimation_algorithms_for_%20(1).pdf --> pg. 4, eq 3
    // See: https://math.stackexchange.com/questions/15323/how-do-i-calculate-the-cartesian-coordinates-of-stars

    // TODO: note about how x = rotmat[2,2], y = [2,1], z = [2,0]

    std::vector<std::vector<long double>> catalog_vectors;

    for (std::tuple<float, long double, long double> star : catalog)
    {
        long double x, y, z;
        x = cosl(std::get<1>(star)) * cosl(std::get<2>(star)); // cos(ra) * cos(dec)
        y = sinl(std::get<1>(star)) * cosl(std::get<2>(star)); // sin(ra) * cos(dec)
        z = sinl(std::get<2>(star));                           // sin(dec)

        catalog_vectors.push_back({x, y, z});
    }

    /*
        Check for double stars

        If two stars are very close to each other (ie pretty much the same Ra and Dec) then the polygons that are formed using them will be almost identical --> not good for identification.
    */

    int n = static_cast<int>(catalog_vectors.size()); // Number of stars in catalogue

    out_stream << "Entries Loaded: " << n << std::endl
               << std::endl;

    int polygon_stars_per_fov = 10; // Max num of stars in the FOV
    int tmp_max = 20;               // Temp max, used to filter later
    std::vector<bool> keep_p(n, false);
    std::vector<bool> keep_t(n, false);
    keep_p[0] = true; // Keep the first star
    keep_t[0] = true;

    for (int star_id = 1; star_id < n; star_id++) // Iterate through stars
    {
        std::vector<long double> vec = catalog_vectors[star_id];

        /*    Calculate the angle to all stars currently in the keep vector (keep_p)    */

        std::vector<long double> vec_p;
        for (int i = 0; i < static_cast<int>(keep_p.size()); i++)
        {
            if (keep_p[i])
            {
                vec_p.push_back(dotProduct_3D(vec, catalog_vectors[i]));
            }
        }

        /*    Calculate the angle to all stars currently in the temporary keep vector (keep_t)    */

        std::vector<long double> vec_t;
        for (int i = 0; i < static_cast<int>(keep_t.size()); i++)
        {
            if (keep_t[i])
            {
                vec_t.push_back(dotProduct_3D(vec, catalog_vectors[i]));
            }
        }

        /*    Check that the star isn't too close to any other stars    */

        bool all_p = true;

        for (int i = 0; i < static_cast<int>(vec_p.size()); i++)
        {
            if (vec_p[i] > cosl(min_star_angle))
                all_p = false;
        }

        if (all_p)
        {
            /*    Count how many stars within the FOV    */

            int count = 0;
            for (int i = 0; i < static_cast<int>(vec_p.size()); i++)
            {
                if (vec_p[i] > cosl(max_fov / 2))
                    count++;
            }

            if (count < polygon_stars_per_fov)
            {
                keep_p[star_id] = true;
                keep_t[star_id] = true;
            }
        }

        /*    Check that the star isn't too close to any other stars in the temp vector    */

        bool all_t = true;

        for (int i = 0; i < static_cast<int>(vec_p.size()); i++)
        {
            if (vec_p[i] > cosl(min_star_angle))
                all_t = false;
        }

        if (all_t)
        {
            /*    Count how many stars within the FOV    */

            int count = 0;
            for (int i = 0; i < static_cast<int>(vec_t.size()); i++)
            {
                if (vec_t[i] > cosl(max_fov / 2))
                    count++;
            }

            if (count < tmp_max)
            {
                keep_t[star_id] = true;
            }
        }
    }

    /*    Extract vectors for all stars to be kept    */

    std::vector<std::vector<long double>> new_catalog_vectors;

    for (int i = 0; i < n; i++)
    {
        if (keep_t[i])
        {
            new_catalog_vectors.push_back(catalog_vectors[i]);
        }
    }

    /*    Update the indexes for the stars    */

    int tmp = -1;

    std::vector<int> new_indexes;

    for (int i = 0; i < static_cast<int>(keep_t.size()); i++)
    {
        if (keep_t[i])
        {
            tmp += 1;
        }
        if (keep_p[i])
        {
            new_indexes.push_back(tmp);
        }
    }

    /*    Output all star vectors to a file    */

    if (save_file_vectors)
    {
        std::ofstream out_stream;

        out_stream.open(vectors_filename); // Open stream with filename

        if (!out_stream)
        {
            std::cerr << "Unable to open file: " << vectors_filename << ". Exiting." << std::endl; 
            exit(-1);
        }

        for (std::vector<long double> vector : new_catalog_vectors)
        {
            for (long double num : vector)
            {
                out_stream << num << "  ";
            }

            out_stream << std::endl;
        }

        out_stream.close();
    }

    /*    Generate temporary star map, to find stars that are in the same area of the sky to form polygons    */

    std::map<std::vector<int>, std::vector<int>> star_map;

    /*    Populate star map    */

    for (int id : new_indexes)
    {
        std::vector<long double> vector = new_catalog_vectors[id];

        std::vector<int> hash = {static_cast<int>(truncl((vector[0] + 1) * 4)), static_cast<int>(truncl((vector[1] + 1) * 4)), static_cast<int>(truncl((vector[2] + 1) * 4))}; // Add one so not multiplying by zero, multiply by 4 to form 4 bins. Can increase to increase resolution of map.

        std::vector<int> current = star_map[hash]; // Find all stars currently at this hash.
        current.push_back(id);  // Add to the hash
        star_map[hash] = current;  // Update the map
    }

    /*    Output Star Map To Log File    */

    out_stream << "    TEMPORARY STAR MAP" << std::endl;

    for (auto x : star_map)
    {
        out_stream << "I: ";
        for (auto y : x.first)
        {
            out_stream << y << "  ";
        }
        out_stream << "      O: ";
        for (auto z : x.second)
        {
            out_stream << z << "  ";
        }
        out_stream << "\n\n";
    }

    /*    Search For Nearby Stars    */

    out_stream << std::endl << "    NEARBY STAR SEARCH" << std::endl;

    std::vector<std::vector<int>> db_polygons; // Polygons to be written to the database.

    for (int star_id : new_indexes) // Iterate through stars
    {
        std::vector<long double> vector = new_catalog_vectors[star_id];

        std::vector<int> hash = {static_cast<int>(truncl((vector[0] + 1) * 4)), static_cast<int>(truncl((vector[1] + 1) * 4)), static_cast<int>(truncl((vector[2] + 1) * 4))};

        std::vector<int> current = star_map[hash]; // Get all stars within the same bin (ie. in the same area of the sky)

        current.erase(current.begin()); // Remove the star from the map
        star_map[hash] = current; // Update the map

        std::vector<int> nearby_stars = starSearchByRadius(vector, max_fov, star_map, new_catalog_vectors); // Find all stars within the FOV

        int n_nearby = static_cast<int>(nearby_stars.size()); // Number of stars nearby

        out_stream << "ID: " << star_id << "    Nearby size: " << n_nearby << std::endl;

        if (n_nearby < 3)
            continue; // skip if not enough nearby stars to form a polygon of 4

        /*    Generate all combinations of the stars to form polygons    */

        combination_generator p_combinations(n_nearby, 3);

        while (!p_combinations.completed)
        {
            std::vector<int> polygon = p_combinations.next();

            std::vector<int> star_ids = {star_id, nearby_stars[polygon[0] - 1], nearby_stars[polygon[1] - 1], nearby_stars[polygon[2] - 1]}; // Extract the star ids

            std::vector<std::vector<int>> pairs = generateCombinations(4, 2);
            bool valid = true;

            for (std::vector<int> pair : pairs)
            {
                if (dotProduct_3D(new_catalog_vectors[star_ids[pair[0]]], new_catalog_vectors[star_ids[pair[1]]]) < cosl(max_fov)) // Check that the polygon fits in the FOV
                {
                    valid = false;
                }
            }

            if (valid)
                db_polygons.push_back(star_ids); // If valid, keep the polygon.
        }
    }

    out_stream << std::endl << std::endl << "    BUILDING CATALOGUE" << std::endl;

    /*    Actually build the output catalogue    */

    n = static_cast<int>(db_polygons.size());

    out_stream << "Catalogue Size: " << n << std::endl;

    std::vector<std::vector<int>> output_catalog(n * 2, std::vector<int>(4, 0)); // preallocate a 2D vec of size [2n x 4], filled with 0's.

    for (std::vector<int> polygon : db_polygons) // Iterate through polygons
    {
        std::vector<long double> vector_distances;
        std::vector<int> hash;
        std::vector<std::vector<int>> pairs = generateCombinations(4, 2);

        for (std::vector<int> pair : pairs)
        {
            vector_distances.push_back(pythagoras3D(new_catalog_vectors[polygon[pair[0]]], new_catalog_vectors[polygon[pair[1]]])); // Calculate the distances between each star in the polygon
        }

        std::sort(vector_distances.begin(), vector_distances.end(), inverseSort); // Sort the distances, largest to smallest

        for (int i = 1; i < static_cast<int>(vector_distances.size()); i++) // Divide all the distances by the largest distance, and form hash.
        {
            vector_distances[i] /= vector_distances[0];
            hash.push_back(static_cast<int>(vector_distances[i] * 25)); // Add to hash
        }

        /*    Add to catalogue    */

        int catalog_index = knuthsHash(hash, n * 2); // Find index using hash
        int i = 0;

        while (true)
        {
            int tmp_index = static_cast<int>((catalog_index + static_cast<int>(pow(i, 2))) % (n * 2)); // Quadratic probing
            if (output_catalog[tmp_index][0] == 0)  // If the location is empty, add to catalogue
            {
                output_catalog[tmp_index] = polygon;
                break;
            }
            i++;
        }
    }

    /*    Write file to disk    */

    out_stream << std::endl << std::endl << "    WRITING CATALOGUE" << std::endl;

    if (save_file)
        writeBinaryCatalogue(output_catalog, catalogue_filename, out_stream);

    out_stream.close();
}

std::vector<std::vector<int>> weightedCombinations(int n, int r) // TODO comment better
{
    /*
        Generates all combinations of length r from [0 -> n], weighted so lower numbers are generated more frequently.

        Eg. weightedCombinations(5,3) = [0,1,2,3], [0,1,2,4], [0,1,3,4], ... , [2,3,4,5]

        Inputs:
            n [int], Combinations are generated from 0 to n.
            r [int], Combination length.

        Output:
            combinations [2D vector, int], The generated combinations.
    */

    std::vector<std::vector<int>> combinations;

    if (n < r)
        return combinations;

    std::vector<int> indexes(r + 1);
    std::iota(indexes.begin(), indexes.end(), -1); // Fills the vector with ascending numbers, starting at -1.
    indexes.push_back(n);

    combinations.push_back({indexes.begin() + 1, indexes.end() - 1});

    while (indexes[1] < (n - r))
    {
        for (int i = 1; i <= r; i++)
        {
            indexes[i]++;

            if (indexes[i] < indexes[i + 1])
                break;

            indexes[i] = indexes[i - 1] + 1;
        }

        combinations.push_back({indexes.begin() + 1, indexes.end() - 1});
    }

    return combinations;
}

std::vector<std::vector<long double>> matrixSum(std::vector<std::vector<long double>> a, std::vector<std::vector<long double>> b) // Maybe remove?
{
    /*
        Sums two matrices together, element wise. Assumes the two matrices are of the same order.

        Inputs:
            a [2D vector, long double], Matrix A.
            b [2D vector, long double], Matrix B.

        Output:
            sum [2D vector, long double], Output matrix, equal to A+B.
    */

    std::vector<std::vector<long double>> sum(a);

    for (int i = 0; i < static_cast<int>(a.size()); i++)
    {
        for (int j = 0; j < static_cast<int>(a[0].size()); j++)
        {
            sum[i][j] += b[i][j];
        }
    }

    return sum;
}

std::vector<std::vector<long double>> matrixMultiplication(std::vector<std::vector<long double>> a, std::vector<std::vector<long double>> b)
{
    /*
        Multiplies two matrices in the order AB.

        The number of rows in matrix A must be equal to the number of columns in matrix B.

        Inputs:
            a, [2d vector, long double]. Matrix A
            b, [2d vector, long double]. Matrix B

        Output:
            output, [2d vector, long double]. The output matrix, which has size (columns in a) x (rows in b).
    */

    int a_y = static_cast<int>(a.size());
    int b_y = static_cast<int>(b.size());
    int b_x = static_cast<int>(b[0].size());

    std::vector<std::vector<long double>> output(a_y, std::vector<long double>(b_x, 0)); // Forms matrix of zeros, with size (a_y * b_x).

    for (int i = 0; i < a_y; i++)
    {
        for (int j = 0; j < b_x; j++)
        {
            output[i][j] = 0;

            for (int k = 0; k < b_y; k++)
            {
                output[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return output;
}

std::vector<std::vector<long double>> transposeMatrix(std::vector<std::vector<long double>> matrix)
{
    /*
        Calculates the transpose of the input matrix.

        Inputs:
            matrix, [2d vector, long double]. Input matrix.

        Outputs:
            matrix_T, [2d vector, long double]. Transpose of the input matrix.
    */

    int m = static_cast<int>(matrix.size());
    int n = static_cast<int>(matrix[0].size());

    std::vector<std::vector<long double>> matrix_T(n, std::vector<long double>(m, 0)); // Creates a matrix with 'opposite' dimensions to the input. Eg. if input m x n, output is n x m

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            matrix_T[i][j] = matrix[j][i];
        }
    }

    return matrix_T;
}

long double determinant(std::vector<std::vector<long double>> matrix)
{
    /*
        Calculates the determinant of the input matrix.
        Only works for 3x3 matrices.

        Input:
            matrix, [2d vector, long double]. Input matrix, 3x3.

        Output:
            [long double]. Determinant.
    */

    if (matrix.size() != matrix[0].size() || matrix.size() != static_cast<size_t>(3))
        exit(-1);

    long double a, b, c;

    a = matrix[0][0] * ((matrix[1][1] * matrix[2][2]) - (matrix[1][2] * matrix[2][1]));
    b = matrix[0][1] * ((matrix[1][0] * matrix[2][2]) - (matrix[1][2] * matrix[2][0]));
    c = matrix[0][2] * ((matrix[1][0] * matrix[2][1]) - (matrix[1][1] * matrix[2][0]));

    return (a - b + c);
}

std::vector<std::vector<long double>> calculateRotationMatrix(std::vector<std::vector<long double>> A, std::vector<std::vector<long double>> B)
{
    /*
        Calculates the optimal rotation matrix between the set of vectors A and B.
        The following code is a modified version of the Kabsch-Umeyama algorithm.
        As all vectors in A and B should be centered around the origin, no translation vector should be needed.

        Inputs:
            A [2D vector, long double], The first set of vectors.
            B [2D vector, long double], The second, paired set of vectors.

        Outputs:
            rotation_matrix [2D vector, long double], The matrix that describes the rotation between A and B.

        Notes:
            https://github.com/scipy/scipy/blob/main/scipy/spatial/transform/_rotation.pyx
            https://en.wikipedia.org/wiki/Kabsch_algorithm
    */

    A = transposeMatrix(A);

    std::vector<std::vector<long double>> cross_covariance = matrixMultiplication(A, B); // Calculate the cross-covariance matrix, using the shortcut: C-C = A'B

    int m = static_cast<int>(cross_covariance.size());
    int n = static_cast<int>(cross_covariance[0].size());

    // Load the covariance matrix in a cv::Mat datatype, so it can be passed to the cv::SVD class.
    cv::Mat_<double> covMatrixMat(m, n);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            covMatrixMat.at<double>(j, i) = static_cast<double>(cross_covariance[i][j]);
        }
    }

    // Run the cv::SVD class. This decomposes the covariance matrix into two rotations (pre- and post-), and a translation using Singular Value Decomposition.
    cv::SVD svd(covMatrixMat);

    // Load cv::Mat U and Vt into 2D vectors.
    std::vector<std::vector<long double>> U(3, std::vector<long double>(3, 0));
    std::vector<std::vector<long double>> Vt(3, std::vector<long double>(3, 0));

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            U[i][j] = static_cast<long double>(svd.u.at<double>(i, j));
            Vt[i][j] = static_cast<long double>(svd.vt.at<double>(i, j));
        }
    }

    // Calculate the overall rotation using U and Vt
    std::vector<std::vector<long double>> rotation_matrix = matrixMultiplication(U, Vt);

    // Ensure the determinant is > 0. If not, multiply bottom row by -1 to form a 'correct' rotation matrix.
    long double det = determinant(rotation_matrix);

    if (det < 0)
        for (int i = 0; i < 3; i++)
            rotation_matrix[2][i] *= -1;

    return rotation_matrix;
}

inline std::vector<std::vector<long double>> vectorSort(std::vector<std::vector<long double>> vectors)
{
    /*
        Sort the vectors by their distance to the average vector.

        Inputs:
            vectors [2D vector, long double], The vectors to be sorted.

        Outputs:
            vector_sorted [2D vector, long double], The sorted vector.
    */

    int size = static_cast<int>(vectors.size());

    std::vector<long double> average(3, 0);
    std::vector<std::vector<long double>> vector_sorted(size);
    std::vector<std::pair<long double, std::vector<long double>>> vector_pairs;

    // Average the vectors
    for (int i = 0; i < size; i++)
    {
        average[0] += vectors[i][0];
        average[1] += vectors[i][1];
        average[2] += vectors[i][2];
    }

    for (int i = 0; i < 3; i++)
    {
        average[i] /= size;
    }

    // Calculate the distance from each vector to the average vector
    for (std::vector<long double> vector : vectors)
    {
        vector_pairs.push_back(std::make_pair(pythagoras3D(vector, average), vector));
    }

    // Sort by this distance, using a lamda sort function.
    std::sort(
        vector_pairs.begin(),
        vector_pairs.end(),
        [](std::pair<long double, std::vector<long double>> a,
           std::pair<long double, std::vector<long double>> b)
        {
            return a.first < b.first;
        });

    // Extract the vectors in the order of distance.
    for (int i = 0; i < size; i++)
    {
        vector_sorted[i] = vector_pairs[i].second;
    }

    return vector_sorted;
}

long double binomialPD(int k, int n, long double p)
{
    /*
        Returns the probability of a certain event using the binomial distribution, using the formula:

            X ~ B(n, p)
            prob = P(X = k)

        Inputs:
            k [int], Number of successes.
            n [int], Number of trials.
            p [long double], Probability of success.

        Outputs:
            prob [long double], Probability of the event occuring.

        Code modified from:
            GSPdibbler, https://stackoverflow.com/questions/22823158/c-binomial-distribution
    */

    long double prob = 1;

    for (int i = n - k + 1; i <= n; ++i)
        prob *= i; 
    for (int i = 1; i <= k; ++i)
        prob /= i;

    prob *= powl(p, k) * powl(1.0 - p, n - k);

    return prob;
}

long double binomialCD(int k, int n, long double p)
{
    /*
        Returns the probability of an event using the cumulative binomial distribution, using the formula:

            X ~ B(n, p)
            prob_out = P(X <= k)

        Inputs:
            k [int], Number of successes.
            n [int], Number of trials.
            p [long double], Probability of success.

        Outputs:
            prob [long double], Probability of the event occuring.
    */

    if (p > 1 || p < 0 || k > n)
        return 1;

    long double prob_out = 0;

    for (int i = 0; i <= k; i++)
    {
        prob_out += binomialPD(i, n, p);
    }

    return prob_out;
}

bool validateMatch(int n_stars, int n_valid, int n_matches, long double &prob)
{
    /*
        Calculates the probability of a match using the binomial distribution, and checks if the probability is within a certain threshold.

        Inputs:
            n_stars [int], Number of stars extracted from the image.
            n_valid [int], Number of stars within the fov bounds.
            n_matches [int], Number of matches between the catalog and image.

        Outputs:
            prob [long double, reference], Probability of a false positive.
            [bool], Is the match within a certain threshold.
    */

    long double prob_false_positive = 1 - (1 - (n_valid * (powl(0.01, 2)))); 

    long double binom_prob_false_positive = binomialCD((n_stars - n_matches - 2), n_stars, (1 - prob_false_positive));

    prob = binom_prob_false_positive;

    if (binom_prob_false_positive < prob_match_threshold) 
    {
        return true;
    }
    return false;
}

void solve(std::vector<std::tuple<int, long double, long double>> star_centroids, long double est_fov /*deg*/, std::vector<std::vector<int>> &catalog, std::vector<std::vector<long double>> &catalog_vectors, std::ofstream &out_stream)
{
    /*
        Main solving function. Calculates

        Inputs:
            star_centroids [vector, tuple], Each element in the vector contains data about

    */

    /*    Set up variables    */
    est_fov = deg_to_rad(est_fov);

    std::vector<std::vector<long double>> full_output_vector;                    // Contains the full output, stored as [[Ra, Dec, Roll, Fov, Prob]]
    std::vector<std::vector<int>> combinations_4C2 = generateCombinations(4, 2); // All combinations of 2 out of 4 --> [0,1], [0,2], ... , [3,4]

    int catalog_size = static_cast<int>(catalog.size());
    int catalog_vectors_size = static_cast<int>(catalog_vectors.size());
    int centroids_size = static_cast<int>(star_centroids.size());

    /*    Iterate through polygons    */

    std::vector<std::vector<int>> polygons = weightedCombinations(centroids_size, 4); // Generates the indexes of the stars to be used to form polygons. Weighted so brighter stars (lower index) are used first.

    for (std::vector<int> polygon : polygons)
    {
        /*    Calculate the 'edges' of the polygon    */

        std::vector<std::tuple<int, long double, long double>> polygon_centroids =
            {
                star_centroids[polygon[0]],
                star_centroids[polygon[1]],
                star_centroids[polygon[2]],
                star_centroids[polygon[3]]}; // Gets the centroids of the stars used in the polygon, using their index.

        std::vector<std::vector<long double>> polygon_vectors = calculateStarVectors(polygon_centroids, est_fov); // Calculates the vectors for each star, using the estimated field of view and the star centroids.
        std::vector<long double> polygon_edges;

        global_polygon_centroids = polygon_centroids; // Global variable used when minimizing the error in the Field of View

        for (std::vector<int> combination : combinations_4C2) // Calculates the 'distance' between star in the polygon, refered to as the edges.
        {
            polygon_edges.push_back(pythagoras3D(polygon_vectors[combination[0]], polygon_vectors[combination[1]]));
        }

        std::sort(polygon_edges.begin(), polygon_edges.end(), inverseSort); // Sort from largest to smallest.

        for (int i = 1; i < 6; i++)
        {
            polygon_edges[i] /= polygon_edges[0]; // Scale each length using the largest edge.
        }

        polygon_edges.erase(polygon_edges.begin()); // Remove the largest, unscaled edge.

        for (std::vector<int> code : GenerateHashCodes(polygon_edges, 0.005, 25)) // Iterate through all possible hash codes.
        {
            /*    Extract all matches    */
            std::vector<std::vector<int>> catalog_matches;
            int count = 0;
            int index = knuthsHash(code, catalog_size); // Calculate the hash index using the hash code.

            while (true) // Use quadratic probing to extract all matches from the catalogue.
            {
                int tmp_index = ((index + static_cast<int>(pow(count++, 2))) % catalog_size);

                if (catalog[tmp_index][0] == 0) // Break when an empty index is found.
                    break;
                catalog_matches.push_back(catalog[tmp_index]);
            }

            if (static_cast<int>(catalog_matches.size()) == 0)
                continue; // If there are no matches, skip to the next hash code.

            for (std::vector<int> match : catalog_matches)
            {
                /*    Iterate through all matches    */

                std::vector<std::vector<long double>> match_vectors;
                std::vector<long double> vector_edges;

                for (int star_id : match)
                    match_vectors.push_back(catalog_vectors[star_id]);

                for (std::vector<int> edge : combinations_4C2)
                {
                    vector_edges.push_back(pythagoras3D(match_vectors[edge[0]], match_vectors[edge[1]]));
                }

                std::sort(vector_edges.begin(), vector_edges.end(), inverseSort);

                global_match_edges = vector_edges;

                for (int i = 1; i < static_cast<int>(vector_edges.size()); i++)
                {
                    vector_edges[i] /= vector_edges[0];
                }

                vector_edges.erase(vector_edges.begin());

                /*    Check error between the polygon and the matched vectors    */

                bool flag = false;

                for (int i = 0; i < static_cast<int>(vector_edges.size()); i++)
                {
                    if (abs(polygon_edges[i] - vector_edges[i]) > 0.007)
                        flag = true; // If tolerance for any value is too great, skip this vector.
                }

                if (flag)
                    continue;

                /*    Calculate the Field of View    */

                // See: https://eigen.tuxfamily.org/dox/unsupported/classEigen_1_1LevenbergMarquardt.html

                Eigen::VectorXd fov_est(1);                // Create a 1D vector using Eigen --> Eigen functors only support Eigen vectors.
                fov_est(0) = static_cast<double>(est_fov); // Asign the estimated Fov to this vector. Cast to double as long doubles not supported by Eigen.

                calculateFov functor;                                                                            // Create a new functor, using the calculateFov struct.
                Eigen::NumericalDiff<calculateFov> diff(functor);                                                // Numerically differentiate the functor using Eigen
                Eigen::LevenbergMarquardt<Eigen::NumericalDiff<calculateFov>, double> least_square_approx(diff); // Minimization class using the least squares error using the Levenberg-Marquardt method.

                least_square_approx.parameters.maxfev = 2000;  // Maximum number of function evaluations.
                least_square_approx.parameters.xtol = 1.0e-10; // Tolerance of solution vector.

                int return_val = least_square_approx.minimize(fov_est); // Minimize the error in fov_est

                long double new_fov = static_cast<long double>(fov_est(0));

                /*    Recalculate vectors with new Fov    */

                std::vector<std::vector<long double>> new_polygon_vectors = calculateStarVectors(polygon_centroids, new_fov);

                /*    Calculate rotation matrix    */

                // Sort the vectors by distance to the middle of the polygon. This order should be the same between the polygon vectors and the match vectors, if the match is correct.
                std::vector<std::vector<long double>> polygon_sorted = vectorSort(new_polygon_vectors);
                std::vector<std::vector<long double>> match_sorted = vectorSort(match_vectors);

                std::vector<std::vector<long double>> rotation_matrix = calculateRotationMatrix(polygon_sorted, match_sorted);

                /*    Recalculate all star vectors, using new Fov and rotation matrix*/

                // Extract center vector
                std::vector<long double> center_vector = {rotation_matrix[0][0], rotation_matrix[1][0], rotation_matrix[2][0]}; // center_vector = [[1],[0],[0]]*rotation_matrix = first column of rotation matrix (As the vector is stored as [z,x,y], the center vector = [1,0,0]*rotation)

                // Recalculate star vectors, and apply rotation
                std::vector<std::vector<long double>> new_star_vectors = calculateStarVectors(star_centroids, new_fov);
                std::vector<std::vector<long double>> corrected_star_vectors(centroids_size, {0, 0, 0});

                for (int i = 0; i < centroids_size; i++)
                {
                    std::vector<std::vector<long double>> tmp_star_vec;

                    for (long double val : new_star_vectors[i])
                        tmp_star_vec.push_back({val}); // Turn vector into 1D matrix

                    std::vector<std::vector<long double>> tmp_corrected = matrixMultiplication(rotation_matrix, tmp_star_vec); // Apply rotation.

                    corrected_star_vectors[i][0] = tmp_corrected[0][0];
                    corrected_star_vectors[i][1] = tmp_corrected[1][0];
                    corrected_star_vectors[i][2] = tmp_corrected[2][0];
                }

                /*    Extract all stars within the diagonal of the screen    */

                long double search_radius = new_fov * (sqrtl(powl(img_width, 2) + powl(img_height, 2))) / (img_width * 2); // Scales the Fov to extract all stars within the diagonal of the screen, not just the width. (As Fov represents the width)

                std::vector<std::vector<long double>> extracted_vectors;

                for (int i = 0; i < catalog_vectors_size; i++) // Iterate through all star vectors in the catalogue
                {
                    if (dotProduct_3D(center_vector, catalog_vectors[i]) > cosl(search_radius)) // If they are within the search radius (ie in the image), add them to extracted_vectors
                    {
                        extracted_vectors.push_back(catalog_vectors[i]);
                    }
                }

                /*    Match the star vectors from the catalogue to the image    */

                int n_extracted = static_cast<int>(extracted_vectors.size());
                std::vector<std::pair<int, int>> match_indexes; // catalog_vectors index, extracted_vectors index

                for (int i = 0; i < centroids_size; i++)
                {
                    int match_id = -1;
                    bool multiple_matches = false;

                    for (int id_2 = 0; id_2 < n_extracted; id_2++)
                    {
                        if (dotProduct_3D(corrected_star_vectors[i], extracted_vectors[id_2]) > cosl(new_fov * 0.01)) // Are the stars the same, +- some error (1% of the Fov)
                        {
                            if (match_id == -1)
                            {
                                match_id = id_2;
                            }
                            else
                            {
                                multiple_matches = true;
                                break;
                            }
                        }
                    }

                    if (!multiple_matches)
                    {
                        if (match_id != -1)
                        {
                            match_indexes.push_back(std::make_pair(i, match_id));
                        }
                    }
                }

                /*    Validate the match    */

                int n_matches = static_cast<int>(match_indexes.size());

                if (n_matches == 0)
                    continue;

                long double prob;

                if (validateMatch(centroids_size, n_extracted, n_matches, prob))
                {
                    /*    Recalculate the rotation matrix    */

                    // Extract the actual matching vectors
                    std::vector<std::vector<long double>> catalog_match_vectors(n_matches);
                    std::vector<std::vector<long double>> valid_match_vectors(n_matches);

                    for (int i = 0; i < n_matches; i++)
                    {
                        catalog_match_vectors[i] = new_star_vectors[match_indexes[i].first];
                        valid_match_vectors[i] = extracted_vectors[match_indexes[i].second];
                    }

                    std::vector<std::vector<long double>> new_rotation_matrix = calculateRotationMatrix(catalog_match_vectors, valid_match_vectors);

                    /*    Extract data from rotation matrix    */

                    // See https://stackoverflow.com/questions/15022630/how-to-calculate-the-angle-from-rotation-matrix

                    long double ra, dec, tmp_dec, roll;

                    ra = atan2l(new_rotation_matrix[1][0], new_rotation_matrix[0][0]);

                    tmp_dec = sqrtl(powl(new_rotation_matrix[2][1], 2) + powl(new_rotation_matrix[2][2], 2));
                    dec = atan2l(new_rotation_matrix[2][0], tmp_dec);

                    roll = atan2l(new_rotation_matrix[2][1], new_rotation_matrix[2][2]);

                    full_output_vector.push_back({rad_to_deg(ra), rad_to_deg(dec), rad_to_deg(roll), rad_to_deg(new_fov), prob});

                    if (fast)
                        goto output_gt; // If only requiring one match, exit loop here.
                }
            }
        }
    }

output_gt:

    if (static_cast<int>(full_output_vector.size()) == 0)
    {
        std::cout << "empty array----" << std::endl;
        return;
    }

    if (full_output)
        out_stream << std::endl
                   << "    FULL OUTPUT" << std::endl;

    std::vector<long double> best_match = full_output_vector[0];

    std::cout << static_cast<int>(full_output_vector.size()) << std::endl;

    for (std::vector<long double> output : full_output_vector)
    {
        if (output[4] < best_match[4])
        {
            best_match = output; // Sort the matches in order of probability.
        }
        if (full_output)
        {
            std::cout << output[0] << " " << output[1] << " " << output[2] << " " << output[3] << " " << output[4] << " " << std::endl;                                      // Output the results in the order: ra dec roll fov prob
            out_stream << "RA: " << output[0] << ", DEC: " << output[1] << ", ROLL: " << output[2] << ", FOV: " << output[3] << ", PROB: " << output[4] << " " << std::endl; // Write to the log file
        }
    }

    // Write to the log file
    out_stream << std::endl
               << "    FINAL RESULT" << std::endl;
    out_stream << "RA: " << best_match[0] << std::endl;
    out_stream << "DEC: " << best_match[1] << std::endl;
    out_stream << "ROLL: " << best_match[2] << std::endl;
    out_stream << "FOV: " << best_match[3] << std::endl;
    out_stream << "PROB: " << best_match[4] << std::endl;

    std::cout << best_match[0] << " " << best_match[1] << " " << best_match[2] << " " << best_match[3] << " " << best_match[4] << " " << std::endl; // Output the best match.
}

cv::Mat imageThreshold(cv::Mat image, int mask_level)
{
    /*
        Calculates a binary mask of the original image, using the mask_level threshold.
        The output will be a black image, with white pixels representing any areas of the original image above the threshold value.

        Inputs:
            image [cv::Mat], The original image.
            mask_level [int], The threshold for the binary mask. 0 <= mask_level <= 255

        Output:
            channels[0] [cv::Mat], One channel of the masked image.
    */

    cv::Mat img;
    std::vector<cv::Mat> channels;

    cv::cvtColor(image, img, cv::COLOR_RGB2GRAY);                // Converts the image to grayscale
    cv::threshold(img, img, mask_level, 255, cv::THRESH_BINARY); // Binary threshold using mask_level
    cv::split(img, channels);                                    // Splits the image into individual channels.

    return channels[0];
}

std::vector<std::vector<int>> recursiveRegionCheck(int x, int y, std::vector<std::vector<int>> regions, int current_region, cv::Mat mask)
{
    /*
        Given the coordinates of a 'bright' pixel, it returns a vector of all 'connected' bright pixels using recursion.

        Inputs:
            x [int], X coordinate of the pixel.
            y [int], Y coordinate of the pixel.
            regions [2D vector, int], Vector of 'bright' pixels.
            mask [cv::Mat], Binary mask of the image.

        Output:
            regions [2D vector, int], Vector of 'bright' pixels.
    */

    if (x > static_cast<int>(regions[0].size()) || x < 1 || y > static_cast<int>(regions.size()) || y < 1) // Base case.
    {
        return regions;
    }

    std::vector<std::vector<int>> positions = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    for (std::vector<int> position : positions) // Checks all pixels surrounding the current pixel, providing they are valid (ie not off the edge of the image.)
    {
        if (mask.at<uchar>((y + position[0]), (x + position[1])) == 255 && regions[(y + position[0])][(x + position[1])] == 0)
        {
            regions[(y + position[0])][(x + position[1])] = current_region;
            regions = recursiveRegionCheck((x + position[1]), (y + position[0]), regions, current_region, mask); // Calls itself for recursion
        }
    }

    return regions;
}

labeled_array LabelRegions(cv::Mat mask)
{
    /*
        Labels regions inside a binary masked image:

            Eg: inputing a mask with:
                [[0,1,1,0],
                [0,1,0,0],
                [0,0,0,1],
                [1,1,0,1]]

            returns:
                [[0,1,1,0],
                [0,1,0,0],
                [0,0,0,2],
                [3,3,0,2]]

        Inputs:
            mask [cv::Mat], Binary mask of the image.

        Outputs:
            output [labeled_array], Labeled array of all regions in the mask, using the format above.

        */

    labeled_array output;
    int current_region = 0;
    std::vector<std::vector<int>> regions(mask.rows, std::vector<int>(mask.cols, 0)); // creates a 2D vectos of same size as the mask, filled with 0's

    for (int y = 0; y < mask.rows; y++) // Iterate through the mask.
    {
        for (int x = 0; x < mask.cols; x++)
        {
            if (regions[y][x] == 0 && mask.at<uchar>(y, x) == 255) // Check if the star pixel has been added to the regions array, and check if the pixel value is equal to 255.
            {
                current_region += 1;
                regions = recursiveRegionCheck(x, y, regions, current_region, mask);
            }
        }
    }
    output.count = current_region;
    output.regions = regions;
    return output;
}

std::vector<std::tuple<int, long double, long double>> extractStars(cv::Mat mask, labeled_array regions)
{
    /*
        Extracts the coordinates and size of the stars using the centroiding formula.

        Inputs:
            mask [cv::Mat], The binary mask of the image
            regions [labeled_array], The extracted regions from the image.

        Outputs:
            valid_stars [vector of tuples], The extracted data, in the form Area, Sx, Sy
    */

    std::vector<std::vector<std::vector<int>>> labeled_coordinates(regions.count, std::vector<std::vector<int>>(2, std::vector<int>(0))); // Creates a 3D array

    for (int row = 0; row < mask.rows; row++)
    {
        for (int col = 0; col < mask.cols; col++)
        {
            if (regions.regions[row][col])
            {
                // Fills the 3D Array with the regions data: [region number][y/x][pixel in region]
                labeled_coordinates[regions.regions[row][col] - 1][0].push_back(row);
                labeled_coordinates[regions.regions[row][col] - 1][1].push_back(col);
            }
        }
    }

    std::vector<std::tuple<int, long double, long double>> valid_stars; // All stars that fit between area constraints.

    for (int i = 0; i < regions.count; i++) // Iterating through the regions to calculate statistics
    {
        int area = labeled_coordinates[i][0].size(); // The number of pixels in each region (3rd dimension in 3D array).

        // Check if the star area fits the size bounds
        if (minimum_star_area && area <= minimum_star_area)
            continue; // If too low, could be noise/hot pixel.
        if (maximum_star_area && area >= maximum_star_area)
            continue; // If too high, could be nebula/satellite/moon/etc.

        long double Sx = 0, Sy = 0;           // Centroids (x and y), calculated to sub-pixel accuracy
        long double total = ((i + 1) * area); // Offset by 1 (to avoid multiplying and dividing by 0)
        long double result_x = 0, result_y = 0;

        for (int count_x = 0; count_x < static_cast<int>(labeled_coordinates[i][1].size()); count_x++)
        {
            result_x += (labeled_coordinates[i][1][count_x] * (i + 1));
        }

        for (int count_y = 0; count_y < static_cast<int>(labeled_coordinates[i][0].size()); count_y++)
        {
            result_y += (labeled_coordinates[i][0][count_y] * (i + 1));
        }

        Sx = result_x / total;
        Sy = result_y / total;

        if (Sx > 0 && Sy > 0 && area > 0)
            valid_stars.push_back(std::make_tuple(area, Sy, Sx)); // Pack the data into a tuple, and append to valid_stars array
    }

    std::sort(valid_stars.begin(), valid_stars.end(), tupleVectorSort); // sort them in order of area (ie. roughly sorted by magnitude), using the tupleVectorSort function above.

    return valid_stars;
}

int main(int argc, char *argv[])
{
    /*
        Takes inputs from the CLI, and parses them into variables. Then calls all relevant functions.

        CLI input should be: ./[executable_name] [img_path] [est_fov] [prob] [thresh] [return-star-ex] [return-full-output]

        Inputs:
            argc [int], Count of all inputs that are passed to the executable.
            argv [char*], Char array of all inputs to the executable. Contains:
                executable_name [string], Name of the executable
                img_path [string], Path to the image to be solved
                log_path [string], Path to where the log file should be written
                est_fov [long double], Estimated field-of-view of the image
                prob [long double], Threshold probability for success
                thresh [int], Threshold for binary mask
                return-star-ex [bool], Return extracted stars
                return-full-output [bool], Return full output

        Outputs:
            0 [int], Returns 0 for success.

    */

    auto time_start = std::chrono::steady_clock::now(); // Start timer

    if (std::stoi(argv[1]))
    {
        /*    Ensure All Arguments Are Passed Through CLI.    */

        if (argc != 7)
        {
            std::cout << "Incorrect number of variables passed through CLI. Found " << argc << ", Expected 7." << std::endl;
            exit(-1); // Quit the program
        }

        /*    Start Log File    */

        std::ofstream out_stream;
        out_stream.open(argv[4]); // Open the log file for writing

        out_stream << "    INPUTS" << std::endl;
        out_stream << "Executable Name: " << argv[0] << std::endl;
        out_stream << "Hipparcos Filepath: " << argv[3] << std::endl;
        out_stream << "Log Filepath: " << argv[4] << std::endl;
        out_stream << "Vector Filepath: " << argv[5] << std::endl;
        out_stream << "Catalogue Filepath: " << argv[6] << std::endl;
        out_stream << "FOV: " << argv[2] << std::endl << std::endl;
        out_stream << "    INPUT CATALOGUE" << std::endl;

        std::string file_location = argv[3];
        std::ifstream file(file_location);

        max_fov = deg_to_rad(std::stold(argv[2]));

        /*    Read Hipparcos Data    */

        std::vector<std::tuple<float, long double, long double>> data = readData(file, out_stream);

        /*    Generate Catalogue    */

        generateDatabase(data, argv[5], argv[6], out_stream);
    }

    else
    {
        /*    Ensure All Arguments Are Passed Through CLI    */

        if (argc != 12)
        {
            std::cout << "Incorrect number of variables passed through CLI. Found " << argc << ", Expected 12." << std::endl;
            exit(-1); // Quit the program
        }

        /*    Load All Variables    */

        cv::Mat image = cv::imread(argv[2]); // Read image into cv::Mat

        long double est_fov = std::stold(argv[6]);
        int mask_level = std::stoi(argv[8]);
        int num_stars_reduced = std::stoi(argv[9]); // Number of stars being passed to solver.
        bool return_stars = (std::stoi(argv[10]));

        full_output = (std::stoi(argv[11]));
        prob_match_threshold = powl(10, std::stold(argv[7]));

        /*    Start Log File    */

        std::ofstream out_stream;
        out_stream.open(argv[3]);

        if (!out_stream)
        {
            std::cerr << "Unable to open log file. Exiting. " << std::endl;
            exit(-1);
        }

        out_stream << "    INPUTS" << std::endl;
        out_stream << "Executable Name: " << argv[0] << std::endl;
        out_stream << "Image Path: " << argv[2] << std::endl;
        out_stream << "Log Path: " << argv[3] << std::endl;
        out_stream << "Estimated FOV: " << argv[6] << std::endl;
        out_stream << "Mismatch Probability: " << argv[7] << std::endl;
        out_stream << "Threshold Value: " << argv[8] << std::endl;
        out_stream << "Return Star Extraction: " << (std::stoi(argv[9]) ? "True" : "False") << std::endl;
        out_stream << "Return All Matches: " << (std::stoi(argv[10]) ? "True" : "False") << std::endl << std::endl;

        /*    Start image processing    */

        img_width = image.cols;
        img_height = image.rows;

        cv::Mat mask = imageThreshold(image, mask_level); // Convert image into binary mask using thresholding.
        labeled_array regions = LabelRegions(mask); // Label regions of 'brightness' in the mask.

        /*    Extract stars from image.    */

        auto time_stars_start = std::chrono::steady_clock::now();

        std::vector<std::tuple<int, long double, long double>> stars(regions.count);
        stars = extractStars(mask, regions); // Extract all stars in the image, and load into a vector of tuples.

        auto time_stars_end = std::chrono::steady_clock::now();

        /*    Output stars    */

        int num_stars = static_cast<int>(stars.size());

        out_stream << "    EXTRACTED STARS" << std::endl;
        out_stream << "No. Extracted: " << num_stars << std::endl;

        if (return_stars)
            std::cout << num_stars << std::endl; // Tell PHP how many stars to expect.

        int x = 0;
        for (auto i : stars)
        {
            out_stream << "ID: " << x++ << ", AREA: " << std::get<0>(i) << ", Sx: " << std::get<1>(i) << ", Sy: " << std::get<2>(i) << std::endl;
            if (return_stars)
                std::cout << x << " " << std::get<0>(i) << " " << std::get<1>(i) << " " << std::get<2>(i) << std::endl; // Only if outputting stars, then in the format: i area Sx Sy
        }

        /*   Reduce the number of extracted stars, fewer stars is faster to run, but less accurate    */

        std::vector<std::tuple<int, long double, long double>> stars_reduced = stars;
        if (num_stars > num_stars_reduced)
            std::vector<std::tuple<int, long double, long double>> stars_reduced(stars.begin(), stars.begin() + num_stars_reduced); // Reduce some of the extracted stars.

        /*    Load the previously generated catalogues.    */

        auto time_polygon_start = std::chrono::steady_clock::now();

        std::vector<std::vector<int>> polygon_catalogue;
        std::string polygon_catalogue_filename = argv[5];
        loadCatalogueFile(polygon_catalogue_filename, polygon_catalogue);

        auto time_polygon_end = std::chrono::steady_clock::now();
        auto time_vector_start = std::chrono::steady_clock::now();

        std::vector<std::vector<long double>> vectors;
        std::string vectors_filename = argv[4];
        loadVectorFile(vectors_filename, vectors);

        auto time_vector_end = std::chrono::steady_clock::now();

        /*    Start the solve    */

        auto time_solve_start = std::chrono::steady_clock::now();

        solve(stars_reduced, est_fov, polygon_catalogue, vectors, out_stream);

        auto time_solve_end = std::chrono::steady_clock::now();

        /*    Log all times    */

        out_stream << std::endl
                   << "    TIMES" << std::endl;
        out_stream << "All times in microseconds" << std::endl;
        out_stream << "Extract Stars: " << std::chrono::duration_cast<std::chrono::microseconds>(time_stars_end - time_stars_start).count() << std::endl;
        out_stream << "Load Polygon Catalogue: " << std::chrono::duration_cast<std::chrono::microseconds>(time_polygon_end - time_polygon_start).count() << std::endl;
        out_stream << "Load Vector Catalogue: " << std::chrono::duration_cast<std::chrono::microseconds>(time_vector_end - time_vector_start).count() << std::endl;
        out_stream << "Solve: " << std::chrono::duration_cast<std::chrono::microseconds>(time_solve_end - time_solve_start).count() << std::endl;
        out_stream << "Total: " << std::chrono::duration_cast<std::chrono::microseconds>(time_solve_end - time_start).count() << std::endl;
    }

    /*    Let PHP know the program is complete    */

    std::cout << "completed" << std::endl;
    return 0;
}
