<!DOCTYPE html>

<html lang="">
    <head>
        <title>Solve</title>
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

        <div class="page-content">
            <div class="solve-container">
                <div class="solve-title">
                    <i class="fa-solid fa-user-astronaut pull-left"></i>
                    <div class="solve-title-container">
                        <h1 class="fa-solid"> &nbsp; Solver</h1>
                        <p class="fa-regular">&nbsp; &nbsp;Start solving now</p>                        
                    </div>
                </div>

                <form action="run_solver.php" enctype="multipart/form-data" method="post">
                    <div class="solve-inputs">
                        <div>
                            <div class="input-container left">
                                <input type="file" class="text-input" id="fileToUpload" name="fileToUpload" required="required">
                                <label for="fileToUpload" class="file-upload"><span id="file-selected" class="file-upload input-hidden"></span></label>
                                <label for="fileToUpload" class="file-upload">Select Image</label>
                                <div class="bar"></div>
                            </div>

                            <div class="input-container right">
                                <input type="number" class="text-input" id="fov" required="required" name="fov" step="0.001">
                                <label for="fov">Estimated FOV (&deg;)</label>
                                <div class="bar"></div>
                            </div>
                        </div>

                        <div>
                            <div class="input-container left solve-button-container">
                                <button class="solve-buttons" type="button" id="toggle-advanced" formnovalidate>
                                    <i class="fa-solid fa-plus" id="advanced-open"></i>
                                    <i class="fa-solid fa-minus icon-hidden" id="advanced-close"></i>
                                    &nbsp; Advanced
                                </button>    
                            </div>

                            <div class="input-container right solve-button-container">
                                <button class="solve-buttons" type="submit" name="solver_submit"><i class="fa-solid fa-check"></i> &nbsp; Submit</button>
                            </div>
                        </div>
                    </div>

                    <div class="solve-inputs solve-advanced solve-advanced-hidden">
                        <div>
                            <div class="input-container left">
                                <input type="number" class="text-input" required="required" name="prob" id="prob" step="1" min="-50" max="0" value="-9">
                                <label for="prob">Probability of Mismatch (x10-n)</label>
                                <div class="bar"></div>
                            </div>

                            <div class="input-container right">
                                <input type="number" class="text-input" required="required" name="thresh" id="thresh" step="1" min="0" max="256" value="128">
                                <label for="thresh">Star Extraction Threshold (0-256)</label>
                                <div class="bar"></div>
                            </div>
                        </div>

                        <div>
                            <div class="left solve-switch-container">
                                <label class="switch">
                                    <input type="checkbox" class="checkbox" name="star_ex">
                                    <span class="slider"></span>
                                    <p class="slider-text">Star Extraction Output</p>
                                </label>
                            </div>

                            <div class="right solve-switch-container">
                                <label class="switch">
                                    <input type="checkbox" class="checkbox" name="all_matches">
                                    <span class="slider"></span>
                                    <p class="slider-text">All Matches</p>
                                </label>
                            </div>
                        </div>
                    </div>
                </form>
            </div>
        </div>
    </body>
</html>