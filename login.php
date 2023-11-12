<?php
    function login($post_email, $post_password)
    {
        session_start();
        $DATABASE_HOST = 'localhost';
        $DATABASE_USER = 'testuser';
        $DATABASE_PASS = 'Passw0rd';
        $DATABASE_NAME = 'plate_solver_web';

        $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);
        
        if (mysqli_connect_errno()) 
        {
            exit('Failed to connect to MySQL: ' . mysqli_connect_error());
        }

        if ($stmt = $con->prepare('SELECT pword, username FROM users WHERE email = ?')) 
        {
            $stmt->bind_param('s', $post_email);
            $stmt->execute();
            $stmt->store_result();

            if ($stmt->num_rows > 0) 
            {
                $stmt->bind_result($password, $username);
                $stmt->fetch();

                if (password_verify($post_password, $password)) 
                {
                    session_regenerate_id();

                    $_SESSION['loggedin'] = TRUE;
                    $_SESSION['name'] = $username;
                    $_SESSION['email'] = $post_email;
                    header("Location: http://127.0.0.1:8000/index.php"); //Redirect to index.php
                } 
                
                else 
                {
                    $_SESSION['login_state'] = FALSE; //wrong password
                }
            } 
            
            else 
            {
                $_SESSION['login_state'] = FALSE; //wrong username
            }

            $stmt->close();
        } 
    }

    function register()
    {
        $DATABASE_HOST = 'localhost';
        $DATABASE_USER = 'testuser';
        $DATABASE_PASS = 'Passw0rd';
        $DATABASE_NAME = 'plate_solver_web';

        $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);

        if ( mysqli_connect_errno() ) {
            exit('Failed to connect to MySQL: ' . mysqli_connect_error());
        }
        
        $stmt1 = $con->prepare('SELECT users.email FROM users WHERE users.email = ?');

        $stmt1->bind_param('s',$_POST['reg_email']);
        $stmt1->execute();
        $stmt1->store_result();

        if ($stmt1->num_rows == 0)
        {
            if ($stmt = $con->prepare('INSERT INTO users (username, forename, surname, pword, email) VALUES (?, ?, ?, ?, ?)')) 
            {
                $reg_password_ref = crypt($_POST['reg_password'], '$5$/3kz.1if!f9ka'); //Second var is the salt --> maybe randomise + store in DB?
                $stmt->bind_param('sssss', $_POST['reg_username'], $_POST['reg_forename'], $_POST['reg_lastname'], $reg_password_ref, $_POST['reg_email']);

                if($stmt->execute())
                {
                    login($_POST['reg_email'], $_POST['reg_password']);
                }

                else 
                {
                    $_SESSION['register_state'] = FALSE;
                }

                $stmt->close();
            }
        }

        else 
        {
            //Make an error --> email already in use...
        }
    }

    if(isset($_POST['register_submit']))
    {
        register();
    }
    
    if(isset($_POST['login_submit']))
    {
        
        login($_POST['email'], $_POST['password']);
    }

    $login_state = 'login_state_hidden';
    if(isset($_SESSION['login_state']))
    {
        $login_state = '';
        unset($_SESSION['login_state']);
    }

    $register_state = 'login_state_hidden';
    if(isset($_SESSION['register_state']))
    {
        $login_state = '';
        unset($_SESSION['register_state']);
    }

?>
    
<!DOCTYPE html>

<html lang="">
    <head>
        <title>Login</title>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link rel="stylesheet" href="login.css">
        <script src="https://kit.fontawesome.com/9e997eccb0.js" crossorigin="anonymous"></script>
    </head>

    <body>
        <script src="main.js"></script>

        <div class="nav-top">
            <div class="menu-top">
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

        <div class="page-content">
            <div class="login-box">
                <div class="login-left">
                    <h1 class="login-title">
                        Login
                    </h1>

                    <div class='login_state <?= $login_state; ?>'>
                        <p>Failed to login, email or password incorrect.</p>
                    </div>

                    <form action="login.php" method="post">
                        <div class="login-inputs">
                            <div>
                                <div class="input-container">
                                    <input type="email" class="text-input" id="email" required="required" name="email">
                                    <label for="email">Email</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="input-container">
                                    <input type="text" class="text-input" id="password" required="required" name="password">
                                    <label for="password">Password</label>
                                    <div class="bar"></div>
                                </div>
                                
                                <div class="button-container">
                                    <input type="submit" name="login_submit" class="submit-button" value="Login">
                                </div>
                            </div>
                        </div>
                    </form>
                </div>

                <div class="login-fade"></div>
  
                <div class="login-right">
                    <h1 class="login-title">
                        Register
                    </h1>

                    <div class='register_state <?= $register_state; ?>'>
                        <p>Failed to register, please check details.</p>
                    </div>

                    <form action="login.php" method="post">
                        <div class="login-inputs">
                            <div>
                                <div class="input-container">
                                    <input type="text" class="text-input" id="reg_username" required="required" name="reg_username">
                                    <label for="reg_username">Username</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="input-container">
                                    <input type="email" class="text-input" id="reg_email" required="required" name="reg_email">
                                    <label for="reg_email">Email</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="input-container">
                                    <input type="text" class="text-input" id="reg_forename" required="required" name="reg_forename">
                                    <label for="reg_forename">Forename</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="input-container">
                                    <input type="text" class="text-input" id="reg_lastname" required="required" name="reg_lastname">
                                    <label for="reg_lastname">Surname</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="input-container">
                                    <input type="text" class="text-input" id="reg_password" required="required" name="reg_password">
                                    <label for="reg_password">Password</label>
                                    <div class="bar"></div>
                                </div>

                                <div class="button-container">
                                    <input type="submit" name="register_submit" class="submit-button" value="Register">
                                </div>
                            </div>
                        </div>
                    </form>
                </div>
            </div>
        </div>
    </body>
</html>