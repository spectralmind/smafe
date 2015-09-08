<?php 

class VersionService extends RestService {

  public function __construct() {
    parent::__construct("get");
  }

  public function performGet($url, $arguments, $accept) {
    $version = new Version($url, $arguments, $accept);
    $response = $version->getResponseObject($this->getResponseContentType());
    $this->sendResponse( $response );
  }
	
}