<?php
    session_start();

    $DATABASE_HOST = 'localhost';
    $DATABASE_USER = 'testuser';
    $DATABASE_PASS = 'Passw0rd';
    $DATABASE_NAME = 'plate_solver_web';

    $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);

    if (mysqli_connect_errno()) {
        exit('Failed to connect to MySQL: ' . mysqli_connect_error());
    }

    $stmt = $con->prepare('
        SELECT solves.solve_ra, solves.solve_dec, solves.img_filename, solves.log_filename FROM solves WHERE solves.email = ?
    ');

    $stmt->bind_param('s', $_SESSION['email']);
    $stmt->execute();
    $stmt->bind_result($ra, $dec, $img_filename, $log_filename);
?>

<!DOCTYPE html>

<html lang="">
    <head>
        <title>History</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="stylesheet" href="index.css">
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

        <div class="page-content">
            <div class="cards">
                <?php
                    $count = 0;

                    while ($stmt->fetch())
                    {
                        $count += 1;

                        echo "<div class='card' style='background-image: url($img_filename);'>
                            <div class='card-cover'>
                                <div class='card-text'>
                                    <br>
                                    <h1>Ra</h1><br>
                                    <p>";
                        echo number_format((float)$ra, 2, '.', ''); //rounding to 2dp
                        echo        "</p><br>
                                    <h1>Dec</h1><br>
                                    <p>";
                        echo number_format((float)$dec, 2, '.', '');
                        echo        "</p><br><br><br>
                                </div>
                                <div class='card-icons'>
                                    <a href='download.php?file=$img_filename' class='fa-regular fa-image'></a>
                                    <a href='download.php?file=$log_filename.txt' class='fa-regular fa-file-lines'></a>
                                </div>
                            </div>
                        </div>";
                    }

                    $stmt->close();

                    if ($count==0)
                    {
                        echo "<div class='card' style='background-color: white;'>
                                <div class='card-text' style='color:black; transform: translateY(35%);'>
                                    <h1>Start Solving</h1><br>
                                    <h1>to see history</h1><br><br><br>
                                </div>
                            </div>";
                    } 
                ?>
            </div>
        </div>
    </body>
</html>