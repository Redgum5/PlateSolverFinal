<?php
    function change()
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

        $stmt = $con->prepare('UPDATE users SET username=?, forename=?, surname=? WHERE email=?');
        $stmt->bind_param('ssss', $_POST['change_username'], $_POST['change_forename'], $_POST['change_lastname'], $_SESSION['email']);

        if(!$stmt->execute())
        {
            exit('An error occurred, please try again');
        }

        $stmt->close();
    }

    if(isset($_POST['change_submit']))
    {
        unset($_POST['change_submit']);
        change();
    }

    unset($_POST['change_submit']);

    session_start();
    $DATABASE_HOST = 'localhost';
    $DATABASE_USER = 'testuser';
    $DATABASE_PASS = 'Passw0rd';
    $DATABASE_NAME = 'plate_solver_web';

    $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);

    if (mysqli_connect_errno()) {
        exit('Failed to connect to MySQL: ' . mysqli_connect_error());
    }

    $stmt = $con->prepare('SELECT username, forename, surname FROM users WHERE email = ?');

    $stmt->bind_param('s', $_SESSION['email']);
    $stmt->execute();

    $stmt->bind_result($username, $forename, $lastname);
    $stmt->fetch();

    $stmt->close();
?>

<!DOCTYPE html>

<html lang="">
    <head>
        <title>Profile</title>
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
                <div class="login-left">
                    <div class='login-inputs'>
                        <h1 class="login-title">
                            Logout
                        </h1>

                        <form action="logout.php" method="post">
                            <div class="input-container">
                                <input type="submit" name="logout" class="submit-button" value="Logout">
                            </div>
                        </form>

                        <h1 class="login-title">
                            Delete Account
                        </h1>
                        
                        <form action="logout.php" method="post">
                            <div class="input-container">
                                <input type="text" class="text-input" id="delete_password" required="required" name="delete_password">
                                <label for="delete_password">Confirm Password</label>
                                <div class="bar"></div>
                            </div>

                            <div class="input-container">
                                <input type="submit" name="delete" class="submit-button" value="Delete">
                            </div>
                        </form>
                    </div>
                </div>

                <div class="login-fade"></div>
                
                <div class="login-right">
                    <h1 class="login-title">
                        Edit Profile
                    </h1>

                    <div class='register_state <?= $register_state; ?>'>
                        <p>Failed to register, please check details.</p>
                    </div>

                    <form action="profile.php" method="post">
                        <div class="login-inputs">
                            <div>
                                <div class="input-container">
                                    <input type="text" class="text-input" id="change_username" required="required" name="change_username" value="<?= $username?>">
                                    <label for="change_username">Username</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="input-container">
                                    <input type="text" class="text-input" id="change_forename" required="required" name="change_forename" value="<?= $forename?>">
                                    <label for="change_forename">Forename</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="input-container">
                                    <input type="text" class="text-input" id="change_lastname" required="required" name="change_lastname" value="<?= $lastname?>">
                                    <label for="change_lastname">Surname</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="button-container">
                                    <input type="submit" name="change_submit" class="submit-button" value="Update">
                                </div>
                            </div>
                        </div>
                    </form>
                </div>
            </div>
        </div>    
    </body>
</html>