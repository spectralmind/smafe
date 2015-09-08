<?php 
$responceContentType = "text/html";

try {
	// set config file
	$CONFIGFILENAME = "../config/smintapi.ini";
	
  // bootstrap the API 
    // defines autoloader
    // creates logger
  require_once "../lib/bootstrapapi.php";

  
  MyLog::printWithDuration("Starting Request: " . RestUtils::getFullUrl($_SERVER));

  // search for Class to handle request
  $apiClass = RestUtils::getApiMethod($_SERVER, $apiConfig["known_api_methods"]);
  
  // hande Request depending on apiClass
  if (strlen($apiClass)>0) {
    // only if a class was identified
    try {
      class_exists($apiClass);
      $service = new $apiClass();
      $service->handleRawRequest($_SERVER, $_GET, $_POST);
      MyLog::printWithDuration("Finished Request: " . RestUtils::getFullUrl($_SERVER));
    } catch (Exception $e) {
      // catch error if the class was not found
      RestUtils::sendResponse(404, $e->getMessage(), $responceContentType );
    }
  }
  // if no class was identified send a 404 responce
  else 
  {
    RestUtils::sendResponse(404);  
  }
} catch (Exception $e) {  
  RestUtils::sendResponse(500, $e->getMessage(), $responceContentType );
}




