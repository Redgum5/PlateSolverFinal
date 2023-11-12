<!DOCTYPE html>

<html lang="">
    <head>
        <title>Results</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="stylesheet" href="solve.css">
        <script src="https://kit.fontawesome.com/9e997eccb0.js" crossorigin="anonymous"></script>
    </head>

    <body>
        <script src="main.js"></script>

        <div class="nav-top">
            <div class="menu-top">
                <div class="left" id="open-nav-left">
                    <i class="fa-solid fa-bars"></i>
                    <span><i class="fa-solid fa-bars"></i></span>
                </div>

                <div class="left">
                    <i class="fa-solid fa-user-astronaut"></i>
                    <p class="fa-solid" style="font-size: 70%;"> &nbsp; PlateSolver</p>
                    <span>
                        <a href="index.php">
                            <i class="fa-solid fa-user-astronaut"></i>
                            <p class="fa-solid" style="font-size: 70%; color: #FCA311;"> &nbsp; PlateSolver</p>
                        </a>
                    </span>
                </div>

                <div class="right">
                    <i class="fa-solid fa-user"></i>
                    <span><a class="fa-solid fa-user" href='profile.php'></a></span>
                </div>
            </div>
        </div>

        <div class="nav-cover navbar-cover-hidden" id="close-nav-left-cover"></div>

        <div class="nav-side navbar-hidden">
            <div class="menu-side">
                <div class="top">
                    <i class="fa-solid fa-user-astronaut fa-large logo"></i>                    
                </div>
                
                <div class="top hoverable">
                    <a href="solve.php">
                        <i class="fa-solid fa-magnifying-glass"></i>
                        <p class="fa-solid"> &nbsp; Solve</p>
                    </a>
                </div>

                <div class="top hoverable">
                    <a href="history.php">
                        <i class="fa-solid fa-clock-rotate-left"></i>
                        <p class="fa-solid"> &nbsp; history</p>
                    </a>
                </div>
                
                <div class="bottom hoverable" id="close-nav-left">
                    <i class="fa-solid fa-xmark"></i>
                    <p class="fa-solid"> &nbsp; Close</p>
                </div>
            </div>
        </div>
        
        <?php 
            function solver_upload_image($file_name, $server_file_path)
            {
                $file_name = basename($file_name);
            
                $user_dir = "uploads/"; 
                $target_file_path = $user_dir . $file_name;
            
                $img_check_flag = 1;
            
                try
                {
                    if (!getimagesize($server_file_path))
                    {
                        $img_check_flag = 0;
                        echo "Upload failed.";
                    }
                }

                catch(Exception $e)
                {
                    $img_check_flag = 0;
                    echo 'Error: ',  $e->getMessage(), "\n";
                }

                if ($img_check_flag == 1)
                {
                    if (move_uploaded_file($server_file_path, $target_file_path))
                    {
                        echo "The file ". htmlspecialchars($file_name). " has been uploaded.";
                    }
                }
            
                return $target_file_path;
            }

            session_start();
            
            //Find next solve_id in database.
            $DATABASE_HOST = 'localhost';
            $DATABASE_USER = 'testuser';
            $DATABASE_PASS = 'Passw0rd';
            $DATABASE_NAME = 'plate_solver_web';
            $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);
            
            if (mysqli_connect_errno()) {
                exit('Failed to connect to MySQL: ' . mysqli_connect_error());
            }

            $stmt = $con->prepare('
                SELECT AUTO_INCREMENT FROM information_schema.TABLES WHERE TABLE_SCHEMA = ? AND TABLE_NAME = "solves"
            ');

            $stmt->bind_param('s', $DATABASE_NAME);
            $stmt->execute();

            $stmt->bind_result($next_solve_id);
            $stmt->fetch();

            $stmt->close();
            
            //Assign post variables to php variables
            $file_name = 'img' . $next_solve_id . ".png";
            $file_tmp_path = $_FILES["fileToUpload"]["tmp_name"];

            $file_path = solver_upload_image($file_name, $file_tmp_path);
            $log_file_path = 'logs/log'.$next_solve_id;

            $est_fov = $_POST["fov"]; //estimated FOV of the image (x-axis)
            $prob = $_POST["prob"]; //probability threshold for a match to be valid
            $thresh = $_POST["thresh"]; //threshold used to extract stars from other bright objects
            $return_star_ex = ($_POST["star_ex"] == "on") ? 1 : 0; //bool return the positions of extracted stars (true) or nothing (false).
            $return_all_matches = ($_POST["all_matches"] == "on") ? 1 : 0; //bool return all matches (true) or just the most likely match (false).
            
            //Run the platesolver executable using shell_exec, and save the output to a variable
            $solver_output = shell_exec("./PlateSolver_Rewrite '$file_path' '$log_file_path.txt' $est_fov $prob $thresh $return_star_ex $return_all_matches");
            
            //$split output by EOL/New Line, and then split into the different output sections
            $solver_output_split = preg_split('#[\r\n]+#', trim($solver_output));

            $star_data = array();
            $star_data_length = 0;

            if ($return_star_ex)
            {
                $star_data_length = intval($solver_output_split[0]);                
                $star_data = array_slice($solver_output_split, 1, $star_data_length , false);
            }

            $full_output = array();
            $full_output_length = 0;

            if ($return_all_matches)
            {
                $full_output_length = intval($solver_output_split[$star_data_length + 1]);
                $full_output = array_slice($solver_output_split, $star_data_length + 2, $full_output_length, false);
            }

            $best_match = $solver_output_split[count($solver_output_split) - 2];
            
            list($solved_ra, $solved_dec, $solved_roll, $solved_fov, $solved_prob) = explode(" ", $best_match);

            //Insert into db
            $stmt = $con->prepare('
                INSERT INTO solves (email, solve_ra, solve_dec, img_filename, log_filename) VALUES (?, ?, ?, ?, ?)
            ');

            $stmt->bind_param('sddss', $_SESSION['email'], $solved_ra, $solved_dec, $file_path, $log_file_path);

            $stmt->execute();
            $stmt->close();
        ?>
        
        <div class="page-content">
            <div class="php-container">
                <div class="php-result-container">
                    <div class="php-input-image">
                        <img src="uploads/<?php echo $file_name; ?>" alt="uploaded_img">
                    </div>

                    <div class="php-text">
                        <div class="php-inputs">
                            <h1 class="php-head">PlateSolver Inputs</h1>

                            <table class="php-input-table">
                                <tr>
                                    <td><span>Image Uploaded </span></td>
                                    <td><span>Estimated FOV </span></td>
                                </tr>

                                <tr>
                                    <td>&nbsp;&nbsp; <?=$_FILES["fileToUpload"]["name"];?></td>
                                    <td>&nbsp;&nbsp; <?= $_POST["fov"]; ?></td>
                                </tr>

                                <tr>
                                    <td><span>Mismatch Probability </span></td>
                                    <td><span>Star Extraction Threshold </span></td>
                                </tr>

                                <tr>
                                    <td>&nbsp;&nbsp; <?= $_POST["prob"]; ?> </td>
                                    <td>&nbsp;&nbsp; <?= $_POST["thresh"]; ?></td>
                                </tr>

                                <tr>
                                    <td><span>Star Extraction Output </span></td>
                                    <td><span>All Matches </span></td>
                                </tr>

                                <tr>
                                    <td>&nbsp;&nbsp; <?= ($_POST["star_ex"] == "on") ? "True" : "False"; ?></td>
                                    <td>&nbsp;&nbsp; <?= ($_POST["all_matches"] == "on") ? "True" : "False"; ?></td>
                                </tr>
                            </table>
                        </div>

                        <div class="php-bar"></div>

                        <div class="php-outputs">
                            <h1 class="php-head">PlateSolver Output</h1>

                            <table class="php-input-table">
                                <tr>
                                    <td><span>Right Ascension (&deg;)</span></td>
                                    <td><span>Declination (&deg;)</span></td>
                                </tr>

                                <tr>
                                    <td>&nbsp;&nbsp; <?= $solved_ra;?></td>
                                    <td>&nbsp;&nbsp; <?= $solved_dec; ?></td>
                                </tr>

                                <tr>
                                    <td><span>Roll (&deg;)</span></td>
                                    <td><span>Calculated FOV (&deg;)</span></td>
                                </tr>

                                <tr>
                                    <td>&nbsp;&nbsp; <?= $solved_roll; ?> </td>
                                    <td>&nbsp;&nbsp; <?= $solved_fov; ?></td>
                                </tr>

                                <tr>
                                    <td><span>Match Probability </span></td>
                                    <td><span></span></td>
                                </tr>

                                <tr>
                                    <td>&nbsp;&nbsp; <?= $solved_prob; ?></td>
                                    <td></td>
                                </tr>
                            </table>
                        </div>
                    </div>
                </div>

                <div class="php-result-container-dropdown php-dropdown-hidden <?= ($return_star_ex) ? "" : " no-display"; ?>" id="php-dropdown-top">
                    <div class="php-dropdown-title">
                        <h1 class="php-head">Extracted Stars</h1>
                        <div class="php-dropdown-button" id="php-dropdown-button-top">
                            <i class="fa-solid fa-plus" id="php-dropdown-button-top-open"></i>
                            <i class="fa-solid fa-minus php-icon-hidden" id="php-dropdown-button-top-close"></i>
                        </div>
                    </div>

                    <div class="php-dropdown-content" >
                        <div class="php-dropdown-scrollable">
                            <p>
                                <?php 
                                    for ($i = 0; $i < $star_data_length; $i++)
                                    {
                                        $tmp_data = explode(" ", $star_data[$i]);
                                        echo "ID: $tmp_data[0], Area: $tmp_data[1], Sx: $tmp_data[2], Sy: $tmp_data[3]<br>";
                                    }
                                ?>
                            </p>
                        </div>
                    </div>
                </div>

                <div class="php-result-container-dropdown php-dropdown-hidden <?= ($return_all_matches) ? "" : " no-display"; ?>" id="php-dropdown-bottom">
                    <div class="php-dropdown-title">
                        <h1 class="php-head">Full Output</h1>
                        <div class="php-dropdown-button" id="php-dropdown-button-bottom">
                            <i class="fa-solid fa-plus" id="php-dropdown-button-bottom-open"></i>
                            <i class="fa-solid fa-minus php-icon-hidden" id="php-dropdown-button-bottom-close"></i>
                        </div>
                    </div>

                    <div class="php-dropdown-content" >
                        <div class="php-dropdown-scrollable">
                            <p>
                                <?php 
                                    for ($i = 0; $i < $full_output_length; $i++)
                                    {
                                        $tmp_data = explode(" ", $full_output[$i]);
                                        echo "Ra: $tmp_data[0], Dec: $tmp_data[1], Roll: $tmp_data[2], FOV: $tmp_data[3], Prob: $tmp_data[4]<br>";
                                    }
                                ?>
                            </p>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </body>
</html>