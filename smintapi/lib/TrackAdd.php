<?php 

class TrackAdd extends RestXMLObject {
  
  
  protected $fileURL;
  protected $localFile;

  protected $external_key;
  protected $collection; 
  protected $uniqueid;


  
  public function __construct($url, $arguments, $accept) {
    parent::__construct($url, $arguments, $accept);

	$this->fileURL = $this->getRequestArgument("url", null, "string");
	$this->fileURL = $this->getRequestArgument("file", $this->fileURL, "string");
	$this->external_key = $this->getRequestArgument("external_key", null, "string");
	$this->collection = $this->getRequestArgument("collection", $this->apiConfig["databaseconstants"]["defaultCollection"] , "string");
	
	
	$this->download(); // download file if it is not local 
	
	if ( !is_null($this->localFile) ) {
	  // if the file is local or could be downloaded add a job 
  	$this->addJob(); // add a job to the smafe job table
	}
	else
	{
	  $this->response->setStatus(400); 
	  $msg = "File could not be downloaded: $this->fileURL ! "; 
	  $this->errorMessage=$msg;
	}

  }

  protected function download(){
    $filename = $this->fileURL; 
    $this->localFile = null; 
    
    if (file_exists($filename)) {  
      // local file 
        $this->localFile = $this->fileURL;
    } else {
      // might be remote file 
      // try to download 
      
      // check if remote location exists 
      if ( RestUtils::url_exists($this->fileURL) ) {
        $tmpName = tempnam(sys_get_temp_dir(), 'smafe_'); 
        $tmpFilename = $tmpName . '_' . basename($this->fileURL);
        $success = copy($this->fileURL, $tmpFilename);
        if ($success) {
          $this->localFile = $tmpFilename;
          
          //log the file download and create a local urlinfo file
          MyLog::printWithDuration("Downloaded URL: $this->fileURL to Local File: $this->localFile");
          $handle = fopen($tmpName, "w");
          fwrite($handle, "local     : $this->localFile\n");
          fwrite($handle, "url       : $this->fileURL\n");
          fwrite($handle, "ext key   : $this->external_key\n");
          fwrite($handle, "collection: $this->collection\n");
          fclose($handle);
        }
      }
    }
    
	}


  protected function addJob(){
    $dbconnection = $this->dbconnection;
    
    $this->uniqueid = SmafeApiUtils::getUUID();
    
    $sql = "INSERT INTO smafejob_addfile(file_uri, external_key, collection_name, guid) VALUES (:uri, :ext_key, :collection, :guid)";
    $query = $dbconnection->prepare($sql);
    $result = $query->execute(array(
      ':uri'=>$this->localFile,
      ':ext_key'=>$this->external_key,
      ':collection'=>$this->collection,
      ':guid'=>$this->uniqueid,
      ));
    
    if ($result === true) {
      $this->response->setStatus(200);
    }
    else {
      // adding failed
      $this->response->setStatus(403); 
      $this->errorMessage = implode(", ", $query->errorInfo());
    }
    
	}

  
  protected function getXML(){
    if ($this->response->getStatusCode() == 200) {
      $content = SmintXML::simpleXMLElement();

      $track_added = $content->addChild("track_added");
      $track_added->addAttribute("smint_track_id", $this->uniqueid);
      $track_added->addAttribute("url", $this->fileURL);
      $track_added->addAttribute("file", $this->localFile);
  	  if ( !is_null($this->external_key) ) $track_added->addAttribute("external_key", $this->external_key);

  	  return $content->asXML();
	  }
    else 
    {
      if ( !isset($this->errorMessage )) {
        $this->errorMessage = "No additional Information on error available.";
      }
      return SmintErrorXML::error($this->response->getStatusCode(), $this->errorMessage);
    } 
	
	}  
}