<?php 
    session_start();
    $loggedin = false;
    if (isset($_SESSION['loggedin']))
    {
        $loggedin = true;
    }
?>

<!DOCTYPE html>

<html lang="">
    <head>
        <title>PlateSolver</title>
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
                        <a href="#">
                            <i class="fa-solid fa-user-astronaut"></i>
                            <p class="fa-solid" style="font-size: 70%; color: #FCA311;"> &nbsp; PlateSolver</p>
                        </a>
                    </span>
                </div>

                <div class="right">
                    <i class="fa-solid fa-user"></i>
                    <span><a class="fa-solid fa-user" href='<?= ($loggedin) ? 'profile.php' : 'login.php'?>'></a></span>
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
                    <a href="<?= ($loggedin) ? 'solve.php' : 'login.php'?>">
                        <i class="fa-solid fa-magnifying-glass"></i>
                        <p class="fa-solid"> &nbsp; Solve</p>
                    </a>
                </div>

                <div class="top hoverable">
                    <a href="<?= ($loggedin) ? 'history.php' : 'login.php'?>">
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
            <div class="section section1">
                <div class="section1-text">
                    <div class="section1-title">
                        <i class="fa-solid fa-user-astronaut"></i>
                        <p class="fa-solid"> &nbsp; PlateSolver</p>
                    </div>
                    
                    <div class="section1-desc">
                        <br>
                        <p class="fa-solid">A simple web interface for a simple plate solving application.</p>
                    </div>
                </div>
            </div>
            
            <div class="section section2">
                <div class="section2-container">
                    <div class="section2-left">
                        <img src="site_media/place_holder.jpg" alt="tmp_img"></img>
                    </div>

                    <div class="section2-right">
                        <div class="section2-text">
                            <p class="section2-title fa-solid">Get started</p>
                            <br>

                            <p class="section2-desc fa-solid">As simple as</p>
                            <br><br>

                            <i class="fa-solid fa-arrow-right"></i>
                            <p class="section2-desc fa-regular"> &nbsp; Shoot</p>
                            <br>

                            <i class="fa-solid fa-arrow-right"></i>
                            <p class="section2-desc fa-regular"> &nbsp; Submit</p>
                            <br>

                            <i class="fa-solid fa-arrow-right"></i>
                            <p class="section2-desc fa-regular"> &nbsp; Solve</p>
                            <br><br>

                            <p class="section2-desc2 fa-light">Simply take a photo of the night sky, upload it to the solver, and wait for results!</p>
                            <br><br>

                            <div class="section2-button-container">
                                <button class="section2-button"><i class="fa-solid fa-user-astronaut"></i>&nbsp; Go To Solver</button>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            
            <div class="section section3">
                <div class="section3-container">
                   <div class="section3-card section3-left">
                        <div class="section3-card-holder">
                            <div class="section3-text">
                                <p class="fa-solid section3-title">Built Using <br> Hipparcos</p>
                                <br><br><br>

                                <i class="fa-solid fa-rocket section3-icon"></i>
                                <br><br><br>

                                <p class="fa-light section3-desc">Completed in 1997, the ESA Hipparcos Catalogue is a record of the positions of over 118,000 stars. This data is used to drive the Platesolver algorithm, allowing for the identification of stars.</p>
                                <br>

                                <a href="https://www.cosmos.esa.int/web/hipparcos" class="section3-credit hoverable">
                                    <i class="fa-solid fa-arrow-right"></i>
                                    <p class="fa-regular"> &nbsp; ESA</p>
                                </a>
                            </div>
                        </div>
                    </div>

                    <div class="section3-card section3-mid">
                        <div class="section3-card-holder">
                            <div class="section3-text">
                                <p class="fa-solid section3-title">source <br>code</p>
                                <br><br><br>

                                <i class="fa-brands fa-github section3-icon"></i>
                                <br><br><br>
                            
                                <p class="fa-light section3-desc">Primarily built using C++, the core algorithim includes everything from star extraction to pattern recognition. Find out how it works below.</p>
                                <br>

                                <a href="https://github.com/" class="section3-credit hoverable">
                                    <i class="fa-solid fa-arrow-right"></i>
                                    <p class="fa-regular"> &nbsp; Github</p>
                                </a>
                            </div>
                        </div>
                    </div>

                    <div class="section3-card section3-right">
                        <div class="section3-card-holder">
                            <div class="section3-text">
                                <p class="fa-solid section3-title">Webpage Using <br> PHP</p>
                                <br><br><br>

                                <i class="fa-solid fa-code section3-icon"></i>
                                <br><br><br>
                            
                                <p class="fa-light section3-desc">With a PHP frontend and SQL database, it only takes seconds to solve and save your favourite images. Come back at any time to download your photos and results.</p>
                                <br>

                                <a href="https://www.php.net/" class="section3-credit hoverable">
                                    <i class="fa-solid fa-arrow-right"></i>
                                    <p class="fa-regular"> &nbsp; PHP</p>
                                </a>
                            </div>
                        </div>
                    </div>
                </div>              
            </div>

            <div class="section4">
                <div class="contact">
                    <div>
                        <p><i class="fa-regular fa-envelope"></i> notarealemail@gmail.com</p>
                        <p><i class="fa-solid fa-phone"></i> 0123 456 7890</p>
                        <p><i class="fa-solid fa-location-dot"></i> Somewhere over the rainbow</p>
                    </div>

                    <div class="contact-right">
                        <p><i class="fa-solid fa-user-astronaut"></i> Platesolver</p>
                    </div>
                </div>
            </div>
        </div>
    </body>
</html>