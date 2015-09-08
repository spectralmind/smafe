<?php 

class TrackServiceAdd extends RestService {

  public function __construct() {
    parent::__construct("put, post");
  }

  public function performPut($url, $arguments, $accept) {
    $track = new TrackAdd($url, $arguments, $accept);
    $response = $track->getResponseObject($this->getResponseContentType());
    $this->sendResponse( $response );
  }
  

  public function performPost($url, $arguments, $accept) {
    $track = new TrackAdd($url, $arguments, $accept);
    $response = $track->getResponseObject($this->getResponseContentType());
    $this->sendResponse( $response );
  }

	
}