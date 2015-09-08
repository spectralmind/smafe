<?php
require_once 'PHPUnit/Framework.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";

class DeleteUrlTest extends PHPUnit_Framework_TestCase
{ 
  private $config; 
  private $service_url; 
  
  protected function setUp()
  {
    $this->config = TestConfig::getConfig();
    $this->service_url = $this->config['server']['baseurl'].'/track/delete/';
  }

  protected function tearDown()
  {
    // place teardown here
  }
  
  public function testDeleteTrack()
  {

    $ext_key = "url2-C";
    $guid = $this->getGUIDfromExtKey($ext_key);
    
    $this->myFileDeleteTest( $guid, "C", 200, array("track_to_be_deleted"), array("id") ) ; 

  }
  
  private function getGUIDfromExtKey($trackextkey){
    $service_url = $this->config['server']['baseurl'].'/track_external_key/';
    
    $url = $service_url . $trackextkey ; 
    
    $curl = curl_init($url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);  
    curl_setopt($curl, CURLOPT_HTTPHEADER, array ( "Accept: application/xml" ));  
    $curl_response = curl_exec($curl);
    $headers = curl_getinfo($curl);
    curl_close($curl);
    
    if ($curl_response == true) {
      $xml = simplexml_load_string($curl_response);
      MyLog::printWithDuration( "queried with the result: ". str_replace("\n", "", $curl_response) ); 
      return $xml->query['id'];
      
    } else {
      echo __METHOD__ . "\n";
      echo "curl_response:".$curl_response. "\n";      
      print_r($headers);
      $this->fail("There was a problem accessing the api! \n\n" );
    }
    
  }
  
  private function myFileDeleteTest( $guid, $collection, $expectedHTTPStatus="200", $expectedXMLElements, $expectedXMLAttributes ) 
  {
    $url = $this->service_url . "/" . $guid;
    $curl = curl_init($url);
    $curl_post_data = array(
        "collection" => $collection,
        "test" => "test",
        "test2" => "test2",
        );
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);   		// return the result on success 
    curl_setopt($curl, CURLOPT_POSTFIELDS, $curl_post_data);	// set post fields 
    curl_setopt($curl, CURLOPT_HTTPHEADER, array ( "Accept: application/xml" ));  // set accept header
    curl_setopt($curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    $curl_response = curl_exec($curl);
    $headers = curl_getinfo($curl);
    curl_close($curl);

    // print_r($headers);
    //echo $curl_response;
    
    // validate result
    if ($curl_response == true) {
      $xml = SmintTestUtils::validateXMLResponse($headers, $curl_response, "200", array("track_to_be_deleted"), array("collection","id"), $this );
      MyLog::printWithDuration( "Successfully added with the result: ". str_replace("\n", "", $curl_response) ); 
    } else {
      echo __METHOD__ . "\n";
      echo "curl_response:".$curl_response. "\n";      
      print_r($headers);
      print_r($curl_post_data);      
      $this->fail("There was a problem accessing the api! \n\n" );
    }

  }    

  
}
?>

