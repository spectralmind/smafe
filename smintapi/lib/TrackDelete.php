<?php 

class TrackDelete extends RestXMLObject {
  
  
  protected $file_id;
  protected $guid;
  protected $collection; 


  
  public function __construct($url, $arguments, $accept) {
    parent::__construct($url, $arguments, $accept);
  $this->guid = $this->getIDfromURL();
  $this->file_id = $this->getIDfromGUID($this->guid);
	$this->collection = $this->getRequestArgument("collection", $this->apiConfig["databaseconstants"]["defaultCollection"] , "string");
	
	$this->deleteJob(); // add a job to the smafe job table

  }

  protected function getIDfromGUID($guid){
    $dbconnection = $this->dbconnection;
    $sqlquery = "select id from file where guid = :guid";

    // prepare statement
    $stmt = $dbconnection->prepare($sqlquery);
    // bind paramters
    $stmt->bindParam(':guid', $guid, PDO::PARAM_INT);
    $stmt->execute();
    // fetch the results
    $result = $stmt->fetchAll();

    if (array_key_exists(0, $result)) 
      return $result[0][0];
    else 
      return null;
  }  


  protected function deleteJob(){
    $dbconnection = $this->dbconnection;
    
    $sql = "INSERT INTO smafejob_deletefile(file_id, collection_name) VALUES (:file_id, :collection)";
    $query = $dbconnection->prepare($sql);
    $result = $query->execute(array(
      ':file_id'=>$this->file_id,
      ':collection'=>$this->collection,
      ));
    
    if ($result === true) {
      $this->response->setStatus(200);
    }
    else {
      // adding to delete job table  failed
      $this->response->setStatus(403); 
      $this->errorMessage = implode($query->errorInfo());
    }
    
	}

  
  protected function getXML(){
    if ($this->response->getStatusCode() == 200) {
      $content = SmintXML::simpleXMLElement();

      $track_added = $content->addChild("track_to_be_deleted");
      $track_added->addAttribute("id", $this->guid);
      $track_added->addAttribute("collection", $this->collection);

      return $content->asXML();
    }
    else {
      if ( !isset($this->errorMessage )) {
        $this->errorMessage = "No additional Information on error available.";
      }
      return SmintErrorXML::error($this->response->getStatusCode(), $this->errorMessage);
    } 
	
	}  
}