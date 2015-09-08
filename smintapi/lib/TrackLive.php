<?php 

class TrackLive extends RestXMLObject {


  protected $fileURL;
  protected $localFile;
  
  protected $apiversion;

  
  // Array structure containing all details about nearest neighbours that should be 
  // written to XML
  protected $nns;
  

  public function __construct($url, $arguments, $accept) {
    parent::__construct($url, $arguments, $accept);

	$this->fileURL = $this->getRequestArgument("url", "TO_BE_SET", "string");
	//$this->fileURL = $this->getRequestArgument("file", $this->fileURL, "string");	
	$this->apiversion = $this->getRequestArgument("version", 1, "integer");
	
	$maxCount =  ($this->apiConfig["query"]["queryLimit"] <= 0 ) ? PHP_INT_MAX : $this->apiConfig["query"]["queryLimit"] ; 
	$defaultCount =  $this->apiConfig["query"]["queryDefault"]; 
	$requestCount = $this->getRequestArgument("count", $defaultCount ,"integer");
	$this->count = ( $requestCount > $maxCount ) ? $maxCount : $requestCount ;
	
	$this->download(); // download file if it is not local 
	
	if ( ! ($this->apiversion == 1)) {
		$this->response->setStatus(400); 
		$this->errorMessage="Version must be 1! ";
	} else {
		if ( !is_null($this->localFile) ) {
			// if the file is local or could be downloaded add a job
			// do something?
	    
			$this->liveQuery();
		} else {
			$this->response->setStatus(400); 
			$this->errorMessage="File could not be downloaded ! ";
		}
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
        $tmpFilename = tempnam(sys_get_temp_dir(), 'smafe_') . '_' . basename($this->fileURL);
        $success = copy($this->fileURL, $tmpFilename);
        if ($success) {
          $this->localFile = $tmpFilename;
        }
      }
    }
    
	}
	
	

  protected function getURLFromTrack_id($track_id){
    $dbconnection = $this->dbconnection;
    $defaultFVTypeid = $this->apiConfig["query"]["defaultFVTypeid"];
    
    $sqlquery = "select uri from file f, featurevector fv where fv.file_id = f.id and fv.track_id = :track_id and fv.featurevectortype_id = :fvt_id";

    // prepare statement
    $stmt = $dbconnection->prepare($sqlquery);
    // bind paramters
    $stmt->bindParam(':track_id', $track_id, PDO::PARAM_INT);
    $stmt->bindParam(':fvt_id', $defaultFVTypeid, PDO::PARAM_INT);
    $stmt->execute();
	
	// since rowCount() does not work with PDO/Postgres, we do a fetchAll and count the array items
    // not big of a deal since we expect only 1 row anyway.
    $rows = $stmt->fetchAll();
    $rowCount = count($rows);
    

    // check affected rows
    if ($rowCount != 1) {
	    MyLog::printWithDuration("# of returned rows: " . $rowCount);
	    MyLog::printWithDuration("sql command: $sqlquery");
	    MyLog::printWithDuration("parameters: track_id = $track_id, fvt_id = $defaultFVTypeid");
	    return null;
    } else  {
	    // return uri of first (and only)
	    return $rows[0]['uri'];
    }
  }
	
	

 protected function liveQuery() {
    $livehost = $this->apiConfig["liveapi"]["livehost"];
    $liveport = $this->apiConfig["liveapi"]["liveport"];
    $liveclient = $this->apiConfig["liveapi"]["liveclient"];
    $assumedmaxdist = $this->apiConfig["liveapi"]["assumedmaxdist"];

    // build command
    $command = "$liveclient --live --no-daemon --liveport=$liveport --livehost=$livehost --livefile=" . escapeshellarg($this->localFile) . "";
    MyLog::printWithDuration("Command: $command");
    
    $return_var = 0;
    $aOutput = array();
   // passthru ($command);
    exec($command, $aOutput, $return_var); 
    
    if ($return_var == 0) {
	MyLog::printWithDuration(implode(" - ", $aOutput));
	
	//echo "$output";
	//print_r($aOutput);
	
	$this->nns = array();
	
	$c = 0; // counter
	$limit = $this->count;
	  // array of distance values to normalize
	  //$aDistancevalues = array();


	
	foreach ($aOutput as $line)  {
		//echo "$line ";
		// only if we have a line with numbers (excludes the first line with headings)
		$a = explode(',', $line);
		if (is_numeric ($a[0])) { 
			// get URL and add it to array
			array_push($a, $this->getURLFromTrack_id($a[0]));
			// add the record to master array
			array_push($this->nns, $a);
			//array_push($aDistancevalues, $a[2]);
			
			$c++;
			if ($c >= $limit) break;
		}
	}
	
	// normalize distance values between 1 and 0
	//$maxDist = max($aDistancevalues);
	for($i = 0; $i < count($this->nns); $i++) 
//		$this->nns[$i][2] = 1 - $this->nns[$i][2] / $assumedmaxdist;

		$this->nns[$i][2] = max(array(1 - $this->nns[$i][2] / $assumedmaxdist, 0));
	
	/* now, $this->nns contains something like
	
    [0] => Array
        (
            [0] => 149
            [1] => -1
            [2] => 0.909733
            [3] => /home/ewald/tmp/reference_tracks/409601_My Beat_Sumo Acapella.mp3
        )

    [1] => Array
        (
            [0] => 153
            [1] => -1
            [2] => 0.896527166667
            [3] => /home/ewald/tmp/reference_tracks/499135_Finally_Acapella.mp3
        )

    [2] => Array
        (
            [0] => 136
            [1] => -1
            [2] => 0.885311833333
            [3] => /home/ewald/tmp/reference_tracks/289156_Vazilando_Acapella.mp3
        )


	...
	*/
	

//	print_r($this->nns);
	
	//phpinfo();

	$this->response->setStatus(200);

    } else {
	$this->response->setStatus(500);
	$this->errorMessage = "Client exit code = $return_var. Details are logged.";
	 MyLog::printWithDuration(implode(" - ", $aOutput));

    }
  }	

  
  protected function getXML(){
    if ($this->response->getStatusCode() == 200) {
      $content = SmintXML::simpleXMLElement();
      
      foreach ($this->nns as $nn) {
	      $nn_node = $content->addChild("match", $nn[3]);
	      //$nn_node->addAttribute("track_id", $nn[0]);
	      //$nn_node->addAttribute("segment_id", $nn[1]);
	      $nn_node->addAttribute("confidence", number_format($nn[2], 3));
	      //$nn_node->addAttribute("", $nn[1]);
      }

      return $content->asXML();
    } else {
      if ( !isset($this->errorMessage )) {
        $this->errorMessage = "No additional Information on error available.";
      }
      return SmintErrorXML::error($this->response->getStatusCode(), $this->errorMessage);
    } 
	
	}  
}