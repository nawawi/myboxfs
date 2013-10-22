<?
function sendToHost($host,$method,$path,$data,$useragent=0)
{
    // Supply a default method of GET if the one passed was empty
    if (empty($method)) {
        $method = 'GET';
    }
    $method = strtoupper($method);
    $fp = fsockopen($host, 80);
    if ($method == 'GET') {
        $path .= '?' . $data;
    }
    fputs($fp, "$method $path HTTP/1.1\r\n");
    fputs($fp, "Host: $host\r\n");
    fputs($fp,"Content-type: application/x-www-form- urlencoded\r\n");
    fputs($fp, "Content-length: " . strlen($data) . "\r\n");
    if ($useragent) {
        fputs($fp, "User-Agent: MSIE\r\n");
    }
    fputs($fp, "Connection: close\r\n\r\n");
    if ($method == 'POST') {
        fputs($fp, $data);
    }

    while (!feof($fp)) {
        $buf .= fgets($fp,128);
    }
    fclose($fp);
    return $buf;
}

//you will need to setup an array of fields to post with
//then create the post string
$formdata = array ( "x" => "something");
//build the post string
  foreach($formdata AS $key => $val){
   $poststring .= urlencode($key) . "=" . urlencode($val) . "&";
  }
// strip off trailing ampersand
$poststring = substr($poststring, 0, -1);

echo $poststring;

$x = sendToHost('211.24.162.73','GET','/get.php',$poststring."\n");
echo $x;
?>
