<?php 

class ResponseObject {
  private $status, $contentType, $content;
  

   public function __construct() {
     $argv = func_get_args();
     switch( func_num_args() )
       {
       default:
       case 0:
         $this->status = 200;
         $this->contentType = "text/plain";
         $this->content = "no content";
       break;
       case 3:
        self::__construct3( $argv[0], $argv[1], $argv[2] );
       break;
       }
    }
  

  public function __construct3( $status, $contentType, $content ) {
    $this->status = $status;
    $this->contentType = $contentType;
    $this->content = $content;
  }
  
  public function setContent($content){
    $this->content = $content;    
  }

  public function setStatus($status){
    $this->status = $status;    
  }

  public function setContentType($contentType){
    $this->contentType = $contentType;    
  }

  public function getContent(){
    return $this->content;    
  }

  public function getStatusCode(){
    return $this->status;    
  }

  public function getStatusMessage(){
    return self::getStatusCodeMessage($this->status);    
  }

  public function getContentType(){
    return $this->contentType;    
  }
  
  
  public static function getStatusCodeMessage($status)
	{
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
  
}