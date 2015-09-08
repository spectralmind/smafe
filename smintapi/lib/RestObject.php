<?php 

class RestObject {
  protected $url;
  protected $arguments;
  protected $accept;
  protected $apiConfig;
  protected $dbconnection; 
  
  protected $response; 
  
  public function __construct($url, $arguments, $accept) 
  {
    $this->response = new ResponseObject();  
    $this->response->setStatus = 500; // set error code to be overwritten in implementations
    $this->apiConfig = SmintapiConfig::getConfig();
    $this->url = $url;
    $this->arguments = $arguments; 
    $this->accept = $accept;
    $this->dbconnection = SmintapiConfig::getDBConnection();
  }
  
  
  
  public function getResponseObject(){
    throw new Exception('RestObject is abstract and should not be instantiated! Overwrite getResponseObject method!');
  }
  
// possible dataType Values  boolean, integer, float, string, array, object, null
  protected function getRequestArgument($argumentName, $default, $dataType="string") {
    if (array_key_exists($argumentName, $this->arguments) ) {
      $argumentValue = $this->arguments[$argumentName]; 
      if (setType( $argumentValue, $dataType ) ) {
        return $argumentValue;
      } 
    }  
    return $default;
  }

  protected function getIDfromURL() {
    $exploded_url = explode( "/", $this->url ); 
    $track = rawurldecode( array_pop( $exploded_url ) );
    return $track;
  }
  
  
}