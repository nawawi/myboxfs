<html> 
 <head> 
  <title>Virus Information Page</title> 
 </head> 
 <body bgcolor="#FCFCFC"> 
  <h1>Virus Information</h1>
  <p>The URL you requested <?php
if ($_GET) { print $_GET['url']; } ?> 
     is infected by a Virus ( <?php 
if ($_GET) { print $_GET['virus']; } ?>)</br> 
     The Request is blocked</p> 
 </body> 
</html>
