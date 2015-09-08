<?php 
/**
 * Build upon the article: Create a REST API with PHP
 * http://www.gen-x-design.com/archives/create-a-rest-api-with-php/
 */

class RestUtils
{
  public static function getApiMethod($_SERVER, $knownMethods)
  {
	$bestClass = "";
    $path = substr(preg_replace('/\?.*$/', '', $_SERVER['REQUEST_URI']), 1);
    if (count($knownMethods)>0) {
  		foreach ($knownMethods as $knownPath => $knownClass) {
  		  if (!strpos($path, $knownPath) === false) $bestClass = $knownClass;
  		}
  		return $bestClass; 
    } else {
      return "";
    }
  }
  
  public static function getFullUrl($_SERVER) {
    if ( array_key_exists('HTTPS', $_SERVER) && $_SERVER['HTTPS'] == 'on' ) 
      $protocol = 'https';
    else 
      $protocol = 'http'; 

    $location = $_SERVER['REQUEST_URI'];
    if ($_SERVER['QUERY_STRING']) {
      $location = substr($location, 0, strrpos($location, $_SERVER['QUERY_STRING']) - 1);
    }
    return $protocol.'://'.$_SERVER['HTTP_HOST'].$location.$_SERVER['QUERY_STRING'];
  }
  
  public static function getRealIpAddr()
    {
       if (!empty($_SERVER['HTTP_CLIENT_IP']))   //check ip from share internet
        {
          $ip=$_SERVER['HTTP_CLIENT_IP'];
        }
        elseif (!empty($_SERVER['HTTP_X_FORWARDED_FOR']))   //to check ip is pass from proxy
        {
          $ip=$_SERVER['HTTP_X_FORWARDED_FOR'];
        }
        else
        {
          if ( array_key_exists('REMOTE_ADDR', $_SERVER) ) 
            $ip=$_SERVER['REMOTE_ADDR'];
          else 
            $ip='unknown';
        }
        return $ip;
    }
  
  

	public static function sendResponse($status = 200, $body = '', $content_type = 'text/html')
	{
    $status_header = 'HTTP/1.0 ' . $status . ' ' . RestUtils::getStatusCodeMessage($status);
    // set the status
    header($status_header);
    // set the content type
    header('Content-type: ' . $content_type);


		// pages with body are easy
		if($body != '')
		{
			// send the body
			echo $body;
			exit;
		}
		// we need to create the body if none is passed
		else
		{
			// create some body messages
			$message = '';

			// this is purely optional, but makes the pages a little nicer to read
			// for your users.  Since you won't likely send a lot of different status codes,
			// this also shouldn't be too ponderous to maintain
			switch($status)
			{
				case 401:
					$message = 'You must be authorized to view this page.';
					break;
				case 404:
					$message = 'The requested URL ' . $_SERVER['REQUEST_URI'] . ' was not found.';
					break;
				case 500:
					$message = 'The server encountered an error processing your request.';
					break;
				case 501:
					$message = 'The requested method is not implemented.';
					break;
			}

			// servers don't always have a signature turned on (this is an apache directive "ServerSignature On")
			$signature = ($_SERVER['SERVER_SIGNATURE'] == '') ? $_SERVER['SERVER_SOFTWARE'] . ' Server at ' . $_SERVER['SERVER_NAME'] . ' Port ' . $_SERVER['SERVER_PORT'] : $_SERVER['SERVER_SIGNATURE'];

			// this should be templatized in a real-world solution
			$body = '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
						<html>
							<head>
								<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
								<title>' . $status . ' ' . RestUtils::getStatusCodeMessage($status) . '</title>
							</head>
							<body>
								<h1>' . RestUtils::getStatusCodeMessage($status) . '</h1>
								<p>' . $message . '</p>
								<hr />
								<address>' . $signature . '</address>
							</body>
						</html>';

			echo $body;
		}
	}

	public static function getStatusCodeMessage($status)
	{
		// these could be stored in a .ini file and loaded
		// via parse_ini_file()... however, this will suffice
		// for an example
		$codes = Array(
		    100 => 'Continue',
		    101 => 'Switching Protocols',
		    200 => 'OK',
		    201 => 'Created',
		    202 => 'Accepted',
		    203 => 'Non-Authoritative Information',
		    204 => 'No Content',
		    205 => 'Reset Content',
		    206 => 'Partial Content',
		    300 => 'Multiple Choices',
		    301 => 'Moved Permanently',
		    302 => 'Found',
		    303 => 'See Other',
		    304 => 'Not Modified',
		    305 => 'Use Proxy',
		    306 => '(Unused)',
		    307 => 'Temporary Redirect',
		    400 => 'Bad Request',
		    401 => 'Unauthorized',
		    402 => 'Payment Required',
		    403 => 'Forbidden',
		    404 => 'Not Found',
		    405 => 'Method Not Allowed',
		    406 => 'Not Acceptable',
		    407 => 'Proxy Authentication Required',
		    408 => 'Request Timeout',
		    409 => 'Conflict',
		    410 => 'Gone',
		    411 => 'Length Required',
		    412 => 'Precondition Failed',
		    413 => 'Request Entity Too Large',
		    414 => 'Request-URI Too Long',
		    415 => 'Unsupported Media Type',
		    416 => 'Requested Range Not Satisfiable',
		    417 => 'Expectation Failed',
		    500 => 'Internal Server Error',
		    501 => 'Not Implemented',
		    502 => 'Bad Gateway',
		    503 => 'Service Unavailable',
		    504 => 'Gateway Timeout',
		    505 => 'HTTP Version Not Supported'
		);

		return (isset($codes[$status])) ? $codes[$status] : '';
	}
	
	
	
  public static function url_exists($url) {
      // Version 4.x supported
      $handle   = curl_init($url);
      if (false === $handle)
      {
          return false;
      }
      curl_setopt($handle, CURLOPT_HEADER, false);
      curl_setopt($handle, CURLOPT_FAILONERROR, true);  // this works
      curl_setopt($handle, CURLOPT_HTTPHEADER, Array("User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.15) Gecko/20080623 Firefox/2.0.0.15") ); // request as if Firefox    
      curl_setopt($handle, CURLOPT_NOBODY, true);
      curl_setopt($handle, CURLOPT_RETURNTRANSFER, false);
      $connectable = curl_exec($handle);
      curl_close($handle);   
      return $connectable;
  }	
}