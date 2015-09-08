<?php 

class TrackServiceDelete extends RestService {

  public function __construct() {
    parent::__construct("delete");
  }

  public function performDelete($url, $arguments, $accept) {
    $track = new TrackDelete($url, $arguments, $accept);
    $response = $track->getResponseObject($this->getResponseContentType());
    $this->sendResponse( $response );
  }
  
	
}