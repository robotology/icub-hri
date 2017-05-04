#!/bin/sh

FILE="../index.html"

/bin/cat <<EOM >$FILE
<!DOCTYPE HTML>
<html lang="en-US">
    <head>
        <meta charset="UTF-8">
        <meta http-equiv="refresh" content="1;url=https://robotology.github.io/icub-client/doxygen/doc/html/index.html">
        <script type="text/javascript">
            window.location.href = "https://robotology.github.io/icub-client/doxygen/doc/html/index.html"
        </script>
        <title>Page Redirection</title>
    </head>
    <body>
        If you are not redirected automatically, follow the <a href="https://robotology.github.io/icub-client/doxygen/doc/html/index.html">link to the documentation</a>
    </body>
</html>
EOM

