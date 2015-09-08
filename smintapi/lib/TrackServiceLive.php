<?php 

class TrackServiceLive extends RestService {

  public function __construct() {
    parent::__construct("get");
  }

  public function performGet($url, $arguments, $accept) {
    $track = new TrackLive($url, $arguments, $accept);
    $response = $track->getResponseObject($this->getResponseContentType());
    $this->sendResponse( $response );
  }
  
	
}