<?php 

class TrackExternalKey extends Track {
  public function __construct($url, $arguments, $accept) {
    parent::__construct($url, $arguments, $accept);
    
    // set the queryTrackId and queryTrackExternal_Key Values
    $this->queryFileExternal_Key = $this->queryParameterId;
    $this->queryFileId = $this->getIDfromExternalKey($this->queryFileExternal_Key); 
    $this->queryFileGUID = $this->getGUIDfromID($this->queryFileId);

    // always include external_key Values in the Result
    $this->external_keys = true; 
  } 
   
}