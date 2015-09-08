<?php 

class RestService {

  private $supportedMethods;
  
  private static $availableResponseTypes = array(
    "application/xml" =>  "1",
    "text/html"       =>  "1",
    );
  
  public function __construct($supportedMethods) {
    $this->supportedMethods = $supportedMethods;
  }

  private function handleMagicQuotes($requestValues) {
    if(get_magic_quotes_gpc())
      return stripslashes_deep($requestValues);
    else
      return $requestValues;    
  }

  public function handleRawRequest($_SERVER, $_GET, $_POST) {
    $url = $this->getFullUrl($_SERVER);
    $method = $_SERVER['REQUEST_METHOD'];
    switch ($method) {
      case 'GET':
      case 'HEAD':
        $arguments = $this->handleMagicQuotes($_GET);
        break;
      case 'POST':
        $arguments = $this->handleMagicQuotes($_POST);
        break;
      case 'PUT':
      case 'DELETE':
        parse_str(file_get_contents('php://input'), $arguments);
        break;
    }
    $accept = $_SERVER['HTTP_ACCEPT'];
    $this->handleRequest($url, $method, $arguments, $accept);
  }

  protected function getFullUrl($_SERVER) {
    if ( array_key_exists('HTTPS', $_SERVER) && $_SERVER['HTTPS'] == 'on' ) 
      $protocol = 'https';
    else 
      $protocol = 'http'; 

    $location = $_SERVER['REQUEST_URI'];
    if ($_SERVER['QUERY_STRING']) {
      $location = substr($location, 0, strrpos($location, $_SERVER['QUERY_STRING']) - 1);
    }
    return $protocol.'://'.$_SERVER['HTTP_HOST'].$location;
  }

  public function handleRequest($url, $method, $arguments, $accept) {
    switch($method) {
      case 'GET':
        $this->performGet($url, $arguments, $accept);
        break;
      case 'HEAD':
        $this->performHead($url, $arguments, $accept);
        break;
      case 'POST':
        $this->performPost($url, $arguments, $accept);
        break;
      case 'PUT':
        $this->performPut($url, $arguments, $accept);
        break;
      case 'DELETE':
        $this->performDelete($url, $arguments, $accept);
        break;
      default:
        /* 501 (Not Implemented) for any unknown methods */
        header('Allow: ' . $this->supportedMethods, true, 501);
    }
  }

  protected function methodNotAllowedResponse() {
    /* 405 (Method Not Allowed) */
    header('Allow: ' . $this->supportedMethods, true, 405);
  }

  public function performGet($url, $arguments, $accept) {
    $this->methodNotAllowedResponse();
  }

  public function performHead($url, $arguments, $accept) {
    $this->methodNotAllowedResponse();
  }

  public function performPost($url, $arguments, $accept) {
    $this->methodNotAllowedResponse();
  }

  public function performPut($url, $arguments, $accept) {
    $this->methodNotAllowedResponse();
  }

  public function performDelete($url, $arguments, $accept) {
    $this->methodNotAllowedResponse();
  }
  
	public function getPath()
	{
		$path = substr(preg_replace('/\?.*$/', '', $_SERVER['REQUEST_URI']), 1);
		if ($path[strlen($path) - 1] == '/') {
			$path = substr($path, 0, -1);
		}
		return $path;
	}
	
	public function getRequestId()
	{
	  $urlparts = $this->getUrlParts();
	  $countUrlparts = count($urlparts);
	  if ($countUrlparts > 0) {
		  return $urlparts[$countUrlparts-1]; 
		}
		else 
		  return null;
	}
	
	public function getUrlParts()
	{
	  return explode('/', $this->getPath() );
	}
	
	public function getRequestAcceptValues() {
    $AcceptViews = array();
  	foreach(explode(",", $_SERVER['HTTP_ACCEPT']) as $value) {
  		$acceptLine = explode(";", $value);
  		$AcceptViews[$acceptLine[0]] = floatval(array_key_exists(1, $acceptLine) ? substr($acceptLine[1], 2) : 1);
  	}
  	arsort($AcceptViews);	  
  	return $AcceptViews;
	}
	
	public function getResponseContentType($defaultType="text/plain") {
	  $response = array_intersect_key($this->getRequestAcceptValues(), self::$availableResponseTypes);
	  if ( count($response) > 0 ) {
	    return key($response);
	  } else {
	    return $defaultType;
	  }
	}
	
	public function sendResponse( $responseObject )
	{
		$status_header = 'HTTP/1.0 ' . $responseObject->getStatusCode() . ' ' . $responseObject->getStatusMessage();
		// set the status
		header($status_header);

		// set the content type
		header('Content-type: ' . $responseObject->getContentType());

		// send the body
		echo $responseObject->getContent();
	}
	
}