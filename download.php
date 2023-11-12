<?php
    $filepath = $_GET['file'];
    $filename = basename($filepath);

    header('Content-Disposition: attachment; filename=' . $filename);
    readfile($filepath);
    #https://stackoverflow.com/questions/12094080/download-files-from-server-php
?>