<?php
    function createDB()
    {
        session_start();
        $DATABASE_HOST = 'localhost';
        $DATABASE_USER = 'testuser';
        $DATABASE_PASS = 'Passw0rd';
    
        $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS);

        if (mysqli_connect_errno()) {
            exit('Failed to connect to MySQL: ' . mysqli_connect_error());
        }

        $stmt = $con->prepare("
            CREATE DATABASE IF NOT EXISTS plate_solver_web;
        ");

        if(!$stmt->execute())
        {
            exit('An error occurred, please try again');
        }

        $stmt->close();
    }

    function createTables()
    {
        session_start();
        $DATABASE_HOST = 'localhost';
        $DATABASE_USER = 'testuser';
        $DATABASE_PASS = 'Passw0rd';
        $DATABASE_NAME = 'plate_solver_web';
    
        $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);

        if (mysqli_connect_errno()) {
            exit('Failed to connect to MySQL: ' . mysqli_connect_error());
        }

        //Create users table:
        $stmt = $con->prepare("
            CREATE TABLE IF NOT EXISTS users (
                id int(11) NOT NULL AUTO_INCREMENT,
                username varchar(50) NOT NULL,
                forename varchar(50),
                surname varchar(50),
                pword varchar(255) NOT NULL,
                email varchar(100) NOT NULL,
                PRIMARY KEY(id)
            )
        ");

        if(!$stmt->execute())
        {
            exit('An error occurred, please try again');
        }

        //Create solves table
        $stmt = $con->prepare("
            CREATE TABLE IF NOT EXISTS solves (
                solve_id int(11) NOT NULL AUTO_INCREMENT,
                email varchar(100) NOT NULL,
                solve_ra double NOT NULL,
                solve_dec double NOT NULL,
                img_filename varchar(255) NOT NULL,
                log_filename varchar(255) NOT NULL,
                PRIMARY KEY(solve_id)
            )
        ");
        
        if(!$stmt->execute())
        {
            exit('An error occurred, please try again');
        }

        $stmt->close();
    }

    function deleteDB()
    {
        session_start();
        $DATABASE_HOST = 'localhost';
        $DATABASE_USER = 'testuser';
        $DATABASE_PASS = 'Passw0rd';
        $DATABASE_NAME = 'plate_solver_web';
    
        $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);

        if (mysqli_connect_errno()) {
            exit('Failed to connect to MySQL: ' . mysqli_connect_error());
        }

        //Drop table
        $stmt = $con->prepare("
            DROP DATABASE plate_solver_web;
        ");

        if(!$stmt->execute())
        {
            exit('An error occurred, please try again');
        }

        $stmt->close();
    }

    if(isset($_POST['createDB']))
    {
        unset($_POST['createDB']);
        createDB();
        createTables();
    }

    if(isset($_POST['deleteDB']))
    {
        unset($_POST['deleteDB']);
        deleteDB();
    }
?>

<!DOCTYPE html>

<html lang="">
    <head>
        <title>Admin</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="stylesheet" href="login.css">
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
            <div class="login-box">
                <div class='login-left'>   
                    <div class='login-inputs'>
                        <h1 class="login-title">
                            Create DB
                        </h1>
                        
                        <form action="admin.php" method="post">
                            <div class="input-container">
                                <input type="submit" name="createDB" class="submit-button" value="Create">
                            </div>
                        </form>
                    </div>
                </div>

                <div class='login-right'>
                    <div class='login-inputs'>
                        <h1 class="login-title">
                            Delete DB
                        </h1>
                        
                        <form action="admin.php" method="post">
                            <div class="input-container">
                                <input type="text" class="text-input" id="password" required="required" name="password">
                                <label for="password">Password</label>
                                <div class="bar"></div>
                            </div>
                            <div class="input-container">
                                <input type="submit" name="deleteDB" class="submit-button" value="Delete">
                            </div>
                        </form>
                    </div>
                </div>
            </div>
        </div>  
    </body>
</html>