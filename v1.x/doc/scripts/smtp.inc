<?
	define('SMTP_STATUS_NOT_CONNECTED', 1, TRUE);
	define('SMTP_STATUS_CONNECTED', 2, TRUE);
	class smtp{

		var $authenticated;
		var $connection;
		var $recipients;
		var $headers;
		var $timeout;
		var $errors;
		var $status;
		var $body;
		var $from;
		var $host;
		var $port;
		var $helo;
		var $auth;
		var $user;
		var $pass;

		function smtp($params = array()){
			if(!defined('CRLF')) define('CRLF', "\r\n", TRUE);

			$this->authenticated = FALSE;			
			$this->timeout	= 5;
			$this->status = SMTP_STATUS_NOT_CONNECTED;
			$this->host = 'localhost';
			$this->port = 25;
			$this->helo = 'localhost';
			$this->auth = FALSE;
			$this->user = '';
			$this->pass = '';
			$this->errors = array();

			foreach($params as $key => $value){
				$this->$key = $value;
			}
		}

		function &connect($params = array()){

			if(!isset($this->status)){
				$obj = new smtp($params);
				if($obj->connect()){
					$obj->status = SMTP_STATUS_CONNECTED;
				}
				return $obj;
			} else {
				$this->connection = fsockopen($this->host, $this->port, $errno, $errstr, $this->timeout);
				if(function_exists('socket_set_timeout')){
					@socket_set_timeout($this->connection, 5, 0);
				}

				$greeting = $this->get_data();
				if(is_resource($this->connection)){
					return $this->auth ? $this->ehlo() : $this->helo();
				}else{
					$this->errors[] = 'Failed to connect to server: '.$errstr;
					return FALSE;
				}
			}
		}

		function send($params = array()){
			foreach($params as $key => $value){
				$this->set($key, $value);
			}
			if($this->is_connected()){
				if($this->auth AND !$this->authenticated){
					if(!$this->auth())
						return FALSE;
				}

				$this->mail($this->from);
				if(is_array($this->recipients)) {
					foreach($this->recipients as $value)
						$this->rcpt($value);
				} else {
					$this->rcpt($this->recipients);
				}
				if(!$this->data()) return FALSE;

				$headers = str_replace(CRLF.'.', CRLF.'..', trim(implode(CRLF, $this->headers)));
				$body    = str_replace(CRLF.'.', CRLF.'..', $this->body);
				$body    = $body[0] == '.' ? '.'.$body : $body;

				$this->send_data($headers);
				$this->send_data('');
				$this->send_data($body);
				$this->send_data('.');

				$result = (substr(trim($this->get_data()), 0, 3) === '250');
				return $result;
			} else {
				$this->errors[] = 'Not connected!';
				return FALSE;
			}
		}
		
		function helo(){
			if(is_resource($this->connection) && $this->send_data('HELO '.$this->helo) && substr(trim($error = $this->get_data()), 0, 3) === '250' ){
				return TRUE;
			} else {
				$this->errors[] = 'HELO command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}
		
		function ehlo(){
			if(is_resource($this->connection) && $this->send_data('EHLO '.$this->helo) && substr(trim($error = $this->get_data()), 0, 3) === '250' ){
				return TRUE;
			} else {
				$this->errors[] = 'EHLO command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}

		function rset(){
			if(is_resource($this->connection) && $this->send_data('RSET') && substr(trim($error = $this->get_data()), 0, 3) === '250' ){
				return TRUE;

			} else {
				$this->errors[] = 'RSET command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}

		function quit(){
			if(is_resource($this->connection) && $this->send_data('QUIT') && substr(trim($error = $this->get_data()), 0, 3) === '221' ){
				fclose($this->connection);
				$this->status = SMTP_STATUS_NOT_CONNECTED;
				return TRUE;

			} else {
				$this->errors[] = 'QUIT command failed, output: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}
		
		function auth(){
			if(is_resource($this->connection) && $this->send_data('AUTH LOGIN') && substr(trim($error = $this->get_data()), 0, 3) === '334' && $this->send_data(base64_encode($this->user)) && substr(trim($error = $this->get_data()),0,3) === '334' && $this->send_data(base64_encode($this->pass)) && substr(trim($error = $this->get_data()),0,3) === '235' ){
				$this->authenticated = TRUE;
				return TRUE;

			}else{
				$this->errors[] = 'AUTH command failed: ' . trim(substr(trim($error),3));
				return FALSE;
			}
		}

		function mail($from){
			if($this->is_connected() && $this->send_data('MAIL FROM:<'.$from.'>') && substr(trim($this->get_data()), 0, 2) === '250' ){
				return TRUE;
			} else {
				return FALSE;
			}
		}
		
		function rcpt($to){
			if($this->is_connected() && $this->send_data('RCPT TO:<'.$to.'>') && substr(trim($error = $this->get_data()), 0, 2) === '25' ){
				return TRUE;
			} else {
				$this->errors[] = trim(substr(trim($error), 3));
				return FALSE;
			}
		}

		function data(){
			if($this->is_connected() && $this->send_data('DATA') && substr(trim($error = $this->get_data()), 0, 3) === '354' ){
				return TRUE;
			} else {
				$this->errors[] = trim(substr(trim($error), 3));
				return FALSE;
			}
		}

		function is_connected(){
			return (is_resource($this->connection) && ($this->status === SMTP_STATUS_CONNECTED));
		}

		function send_data($data){
			if(is_resource($this->connection)){
				return fwrite($this->connection, $data.CRLF, strlen($data)+2);
				
			} else {
				return FALSE;
			}
		}

		function &get_data(){
			$return = '';
			$line   = '';
			$loops  = 0;
			if(is_resource($this->connection)){
				while((strpos($return, CRLF) === FALSE OR substr($line,3,1) !== ' ') AND $loops < 100){
					$line    = fgets($this->connection, 512);
					$return .= $line;
					$loops++;
				}
				return $return;

			} else {
				return FALSE;
			}
		}

		function set($var, $value){
			$this->$var = $value;
			return TRUE;
		}

	}
?>