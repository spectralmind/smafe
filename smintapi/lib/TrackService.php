<?php 

class TrackService extends RestService {

  public function __construct() {
    parent::__construct("get");
  }

  public function performGet($url, $arguments, $accept) {
    $track = new Track($url, $arguments, $accept);
    $response = $track->getResponseObject($this->getResponseContentType());
    $this->sendResponse( $response );
  }

}