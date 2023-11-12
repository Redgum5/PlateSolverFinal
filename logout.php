<?php
    session_start();

    if (isset($_POST['delete']))
    {
        $DATABASE_HOST = 'localhost';
        $DATABASE_USER = 'testuser';
        $DATABASE_PASS = 'Passw0rd';
        $DATABASE_NAME = 'plate_solver_web';

        $con = mysqli_connect($DATABASE_HOST, $DATABASE_USER, $DATABASE_PASS, $DATABASE_NAME);

        if ( mysqli_connect_errno() ) {
            exit('Failed to connect to MySQL: ' . mysqli_connect_error());
        }

        if ($stmt = $con->prepare('SELECT pword FROM users WHERE id = ?')) {

            $stmt->bind_param('i', $_SESSION['id']);
            $stmt->execute();

            $stmt->store_result();

            if ($stmt->num_rows > 0) {
                $stmt->bind_result($password);
                $stmt->fetch();
                $stmt->close();

                if (password_verify($_POST['delete_password'], $password)) {
                    if($stmt = $con->prepare('DELETE FROM users WHERE id = ?')){
                        $stmt->bind_param('i', $_SESSION['id']);
                        $stmt->execute();
                        $stmt->close();
                    }
                } 
            } 
        }
    }
    
    session_destroy();
    header('Location: index.php');
    
?>