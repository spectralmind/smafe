<?php 

class Track extends RestXMLObject {
  
  protected $queryParameterId; 

  protected $queryFileGUID; 
  protected $queryFileId; 
  protected $queryFileExternal_Key; 
  protected $collection; 
  
  protected $count; 
  protected $distance_values;
  protected $external_keys;
  
  protected $debug;
  
  public function __construct($url, $arguments, $accept) {
    parent::__construct($url, $arguments, $accept);
    $this->queryParameterId = $this->getIDfromURL();
    
    // if the query is a track query the query parameter is a guid 
    $this->queryFileGUID = $this->queryParameterId; 
    
    $maxCount =  ($this->apiConfig["query"]["queryLimit"] <= 0 ) ? PHP_INT_MAX : $this->apiConfig["query"]["queryLimit"] ; 
    $defaultCount =  $this->apiConfig["query"]["queryDefault"]; 
    $requestCount = $this->getRequestArgument("count", $defaultCount ,"integer");
    $this->count = ( $requestCount > $maxCount ) ? $maxCount : $requestCount ;

  	$this->collection = $this->getRequestArgument("collection", $this->apiConfig["databaseconstants"]["defaultCollection"] , "string");

    $this->external_keys = $this->getRequestArgument("external_keys", $this->apiConfig["query"]["externalKeys"], "boolean") ;
    $this->distance_values = $this->getRequestArgument("distance_values", $this->apiConfig["query"]["distanceValues"], "boolean") ;
  }
  
  protected function getXML(){
    $result = $this->queryBySmintFileId();
    if ( is_null($result) ) {
      // if there was an error the result will be null 
      // assume that the detailed error message was set where the query failed
      $this->response->setStatus(403);
      return SmintErrorXML::error($this->response->getStatusCode(), $this->errorMessage);
    }
    
    else if (count($result)>0) {
      $this->response->setStatus(200); 

      $content = SmintXML::simpleXMLElement();

      $querytrack = $content->addChild("query");
      $querytrack->addAttribute("id", $this->queryFileGUID);
      if ($this->external_keys) {
        if (!isset($this->queryFileExternal_Key)) { $this->queryFileExternal_Key = $this->getExternalKeyfromGUID($this->queryFileGUID);}  
        $querytrack->addAttribute("external_key", $this->queryFileExternal_Key );
      }
      if ($this->collection) {
        $querytrack->addAttribute("collection", $this->collection );
      }

      foreach ($result as $trackKey => $trackValue) {
        $resultnode = $querytrack->addChild("result");
        $resultnode->addAttribute("id", $trackValue["guid"]);
        if ($this->external_keys) {
          $resultnode->addAttribute("external_key", $trackValue["external_key"]);
        }
        if ($this->distance_values) {
          $resultnode->addAttribute("value", $trackValue["value"]);
        }
      }

      return $content->asXML();
    } else {
      # check if jobs are already added 
      $job_result = $this->queryJobsBySmintFileId();
      if ( is_null($job_result) ) {
        // if there was an error the result will be null 
        // assume that the detailed error message was set where the query failed
        $this->response->setStatus(403);
        return SmintErrorXML::error($this->response->getStatusCode(), $this->errorMessage);
      }

      if ( count($job_result) > 0 ) {
        $this->response->setStatus(200); 

        $content = SmintXML::simpleXMLElement();

        $querytrack = $content->addChild("query");
        $querytrack->addAttribute("id", $this->queryFileGUID);
        if ($this->external_keys) {
          if (!isset($this->queryFileExternal_Key)) { $this->queryFileExternal_Key = $this->getExternalKeyfromGUID($this->queryFileGUID);}  
          $querytrack->addAttribute("external_key", $this->queryFileExternal_Key );
        }
        if ($this->collection) {
          $querytrack->addAttribute("collection", $this->collection );
        }
        
        if ( $job_result[0]["status"] == null ) {
          $status_message = "waiting for processing.";
          $status_code = 1; 
        } 
        else if ( $job_result[0]["status"] == "FAILED" ) {
          $status_message = "job could not be processed: " . $job_result[0]["log"];
          $status_code = -1; 
        }
        else if ( $job_result[0]["status"] == "OK" ) {
          // check if distjobs are open: 
          $featurevectortype_id = $this->apiConfig["query"]["defaultFVTypeid"];
          $distancetype_id = $this->apiConfig["query"]["defaultDSTypeid"];
          
          if ( $this->openDistanceJobs($job_result[0]["id"],$featurevectortype_id, $distancetype_id) ) {
              $status_message = "processing done. indexing. this might take some time.";
              $status_code = 3; 
          } else {
            $status_message = "job processed. if there is no result, there might be no related track in the database. ";
            $status_code = 0; 
          }
        }
        else if ( strlen($job_result[0]["status"]) > 0 ) {
           // assume that the string is the ID of the daemon performing the current job
          $status_message = "job is processed by process: ". $job_result[0]["status"];
          $status_code = 2; 
        }  
// currently not directly covered by add_job table -> use separate query to check if distance jobs are open         
        // else if ( $job_result[0]["status"] == "DIST" ) {
        //   $status_message = "processing done. indexing might still take some time.";
        //   $status_code = 3; 
        // }  

        $result = $querytrack->addChild("status", $status_message); 
        $result->addAttribute("status_code", $status_code);

        return $content->asXML();
      }
      else {
        $this->response->setStatus(404);
        $this->errorMessage = "Track with the guid: \"$this->queryFileGUID\""; 
        if ($this->external_keys) {
           $this->errorMessage = $this->errorMessage . " / external_key: \"$this->queryFileExternal_Key\"";
        }
        if ($this->collection) {
           $this->errorMessage = $this->errorMessage . " / collection: \"$this->collection\"";
        }
        $this->errorMessage = $this->errorMessage . " could not be found in our database. $this->debug";
        return SmintErrorXML::error($this->response->getStatusCode(), $this->errorMessage);

      }
    }
    
  }
  
  protected function getIDfromExternalKey($external_key){
    $dbconnection = $this->dbconnection;
    $sqlquery = "select id from file where external_key = :external_key";

    // prepare statement
    $stmt = $dbconnection->prepare($sqlquery);
    // bind paramters
    $stmt->bindParam(':external_key', $external_key, PDO::PARAM_STR);
    $stmt->execute();
    // fetch the results
    $result = $stmt->fetchAll();
	if (array_key_exists(0, $result)) return $result[0][0];  
	else return null; 
  }


  protected function getExternalKeyfromGUID($guid){
    $dbconnection = $this->dbconnection;
    $sqlquery = "select external_key from file where guid = :guid";

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
  
  
  protected function getGUIDfromID($id){
    $dbconnection = $this->dbconnection;
    $sqlquery = "select guid from file where id = :id";

    // prepare statement
    $stmt = $dbconnection->prepare($sqlquery);
    // bind paramters
    $stmt->bindParam(':id', $id, PDO::PARAM_INT);
    $stmt->execute();
    // fetch the results
    $result = $stmt->fetchAll();

    if (array_key_exists(0, $result)) 
      return $result[0][0];
    else 
      return null;
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
  
  
  protected function queryBySmintFileId() {
    $featurevectortype_id = $this->apiConfig["query"]["defaultFVTypeid"];
    $distancetype_id = $this->apiConfig["query"]["defaultDSTypeid"];
    $limit = $this->count;
    
    if ( is_null($this->queryFileId) ) $this->queryFileId = $this->getIDfromGUID($this->queryFileGUID);

    // build select statement
    $select_columns = " d.track_b_id, f.external_key, f.guid ";
    $select_tables = " distance d, collection c, collection_file cf, file f, file f2, track t";
      
    $select_where_clause = " c.collection_name=:collection_name AND c.id=cf.collection_id AND cf.file_id=f.id AND f.track_id=t.id AND t.id=d.track_b_id AND d.track_a_id = f2.track_id AND f2.id = :file_id AND d.featurevectortype_id = :featurevectortype_id AND d.distancetype_id = :distancetype_id ";
    if ($this->distance_values) {
      $select_columns = $select_columns . ", d.value ";
      };
  
    $sqlquery = "SELECT ". $select_columns ." FROM ". $select_tables ." WHERE ". $select_where_clause ." ORDER BY value LIMIT :limit";

    $dbconnection = $this->dbconnection;

    // prepare statement
    $stmt = $dbconnection->prepare($sqlquery);
    // bind paramters
    $stmt->bindParam(':file_id', $this->queryFileId, PDO::PARAM_INT);
    $stmt->bindParam(':collection_name', $this->collection, PDO::PARAM_INT);
    $stmt->bindParam(':featurevectortype_id', $featurevectortype_id, PDO::PARAM_INT);
    $stmt->bindParam(':distancetype_id', $distancetype_id, PDO::PARAM_INT);
    $stmt->bindParam(':limit', $limit, PDO::PARAM_INT);
    $querysuccess = $stmt->execute();
    
    if ($querysuccess) {
      // fetch the results
      $result = $stmt->fetchAll(); 
      
      // log debug message
      MyLog::printWithDuration("Query Database with SQL: $sqlquery FileId:$this->queryFileId CollectionId:$this->collection FVTypeId:$featurevectortype_id  DistTypeId:$distancetype_id  Limit:$limit", MyLog::getLevel("DEBUG") ) ;
    }
    else {
      $this->errorMessage = implode(", ", $stmt->errorInfo()) . " querytrackid: $this->queryFileId externalTrackid:  $this->queryFileExternal_Key queryTrackguid: $this->queryFileguid";
      $result = null; 
    }
    return $result;   
  }

  protected function queryJobsBySmintFileId() {
    
    // build select statement
    $select_columns = " id, started, finished, status, log ";
    $select_tables = " smafejob_addfile ";
    if (is_null($this->queryFileGUID)) 
      $select_where_clause = " external_key = :external_key ";
    else
      $select_where_clause = " guid = :guid ";
      
    
    if ( $this->collection != $this->apiConfig["databaseconstants"]["defaultCollection"] ) {
      $select_where_clause = $select_where_clause . " AND collection_name = :collection_name ";
    }
    

    $sqlquery = "SELECT ". $select_columns ." FROM ". $select_tables ." WHERE ". $select_where_clause;

    $dbconnection = $this->dbconnection;

    // prepare statement
    $stmt = $dbconnection->prepare($sqlquery);
    // bind paramters
    if ( $this->collection != $this->apiConfig["databaseconstants"]["defaultCollection"] ) {
      $stmt->bindParam(':collection_name', $this->collection, PDO::PARAM_INT);
    }
    
    if (is_null($this->queryFileGUID)) 
      $stmt->bindParam(':external_key', $this->queryFileExternal_Key, PDO::PARAM_STR);
    else
      $stmt->bindParam(':guid', $this->queryFileGUID, PDO::PARAM_STR);
      
    $querysuccess = $stmt->execute();
    
    if ($querysuccess) {
      // fetch the results
      $result = $stmt->fetchAll(); 
      
      // log debug message
      MyLog::printWithDuration("Query Database with SQL: $sqlquery CollectionId:$this->collection GUID: $this->queryFileGUID ext_key: $this->queryFileExternal_Key", MyLog::getLevel("DEBUG") ) ;
    }
    else {
      $this->errorMessage = implode(", ", $stmt->errorInfo()) . " querytrackid: $this->queryFileId externalTrackid:  $this->queryFileExternal_Key queryTrackguid: $this->queryFileGUID";
      $result = null; 
    }
    return $result;   
  }
  
  protected function openDistanceJobs($addfilejobid, $fvtid, $disttid) {
    $sqlquery = "select count(*) as opendistjobs from distancejob dj where dj.smafejob_addfile_id = :smafejob_addfile_id and dj.featurevectortype_id = :featurevectortype_id and dj.distancetype_id = :distancetype_id;";
    $dbconnection = $this->dbconnection;
    // prepare statement
    $stmt = $dbconnection->prepare($sqlquery);
    // bind paramters
    $stmt->bindParam(':smafejob_addfile_id', $addfilejobid, PDO::PARAM_INT);
    $stmt->bindParam(':featurevectortype_id', $fvtid, PDO::PARAM_INT);
    $stmt->bindParam(':distancetype_id', $disttid, PDO::PARAM_INT);
    // execute query
    $querysuccess = $stmt->execute();
    
    if ($querysuccess) {
      // fetch the results
      $queryresult = $stmt->fetchAll(); 
      if ($queryresult[0][0] > 0) 
        $result = true; 
      else 
        $result = false; 
    }
    else {
      // assume that there are no open tasks if the query was unsuccessful
      $result = false; 
    }
    MyLog::printWithDuration("Query openDistanceJobs $addfilejobid  $fvtid $disttid " . $queryresult[0][0] ) ;
    return $result;   
  }
  
  
}