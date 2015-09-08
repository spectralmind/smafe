<?php
require_once 'PHPUnit/Framework.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";

class AddUrlTest extends PHPUnit_Framework_TestCase
{
  protected $config;
  protected $testfileslocal; 
  protected $testfilesremote; 
  
  protected function setUp()
  {
    $this->config = TestConfig::getConfig();
    $this->testfilesremote = TestConfig::getRemoteTestFiles();
    $this->testfileslocal = TestConfig::getLocalTestFiles();
  }

  protected function tearDown()
  {
    // place teardown here
  }
  
  /**
   * adds all files defined in the array testfilesremote
   *
   * @return void
   * @author jochum
   */
  public function testLoadFileFromURL()
  {
    foreach ($this->testfilesremote as $key => $file) {
      $file_url = $file[0];
      $ext_key =  $file[1];
      $collection = $file[2];

      MyLog::printWithDuration( "Adding URL:$file_url Ext.Key:$ext_key Collection:$collection"); 
      
      $expectedElements = array("track_added"); 
      $expectedAttributes = array( "url" ,"smint_track_id" ); 
      if ( is_string($ext_key) && ( strlen($ext_key) > 0 ) ) 
        $expectedAttributes[] = "external_key"; 

      $this->myFileAddTest($file_url, $ext_key, $collection, 200, $expectedElements, $expectedAttributes);
    }
  }
  
  /**
   * adds all files defined in the array testfileslocal
   *
   * @return void
   * @author jochum
   */
  public function testLoadFileFromLocalFile()
  {
    foreach ($this->testfileslocal as $key => $file) {
      $file_url = dirname(__FILE__) . '/../resources/' . $file[0];
      $ext_key =  $file[1];
      $collection = $file[2];

      MyLog::printWithDuration( "Adding File:$file_url Ext.Key:$ext_key Collection:$collection"); 

      $expectedElements = array("track_added"); 
      $expectedAttributes = array( "file" ,"smint_track_id" ); 
      if ( is_string($ext_key) && ( strlen($ext_key) > 0 ) )
        $expectedAttributes[] = "external_key"; 

      $this->myFileAddTest($file_url, $ext_key, $collection, 200, $expectedElements, $expectedAttributes);
    }
  }

  /**
   * tries to add a file, that does not exist -> will fail, test checks for a failure 
   *
   * @return void
   * @author jochum
   */
  public function testLoadFileFromWrongLocation()
  {

    $file_url = dirname(__FILE__) . '/../resources/doesnotexist.mp3' ;
    $ext_key = "testAddUrlKey_3";

    MyLog::printWithDuration( "Adding File from wrong Location (must fail):$file_url Ext.Key:$ext_key"); 
    $this->myFileAddTest($file_url, $ext_key,"", 400, array("error"), array("code"));
  }

  /**
   * tries to add file with an existing external key -> will fail, test checks for a failure 
   *
   * @return void
   * @author jochum
   */
  public function testLoadFileFromLocalFileDuplicateExternalKeyError()
  {
    
    $file_url = dirname(__FILE__) . '/../resources/' . $this->testfileslocal[5][0];
    $ext_key =  $this->testfileslocal[5][1];
    $collection = $this->testfileslocal[5][2];

    MyLog::printWithDuration( "Adding File with duplicate External Key (must fail):$file_url Ext.Key:$ext_key Collection:$collection"); 
    $this->myFileAddTest($file_url, $ext_key, $collection, 403, array("error", "errorDetail"), array("code"));
  }
  
  
  /**
   * Function to test adding of URLs
   *
   * @param string $file_url the URL 
   * @param string $ext_key the external Key; if empty string no ext_key will be set 
   * @param string $collection the collection name; if empty string no collection will be set 
   * @param string $expectedHTTPStatus the statuscode of the expected result 
   * @param string $expectedXMLElements an array containing all expected elements of the returned xml 
   * @param string $expectedXMLAttributes an array containing all expected attributes of the returned xml 
   * @return void
   * @author jochum
   */
  private function myFileAddTest( $file_url, $ext_key, $collection, $expectedHTTPStatus="200", $expectedXMLElements, $expectedXMLAttributes ) 
  {

    $service_url = $this->config['server']['baseurl'].'/track/add/';

# build post options
    $curl_post_data = array();
    if ( file_exists($file_url) ) {
      # file is local 
      $curl_post_data["file"] = $file_url;
    } else {
      # assume it is remote 
      $curl_post_data["url"] = $file_url;
    }
    
    if ( is_string($ext_key) && ( strlen($ext_key) > 0 ) ) {
      $curl_post_data["external_key"] = $ext_key;
    }    

    if ( is_string($collection) && ( strlen($collection) > 0 ) ) {
      $curl_post_data["collection"] = $collection;
    }    

# build curl request    
    $curl = curl_init($service_url);
    curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);   		// return the result on success 
    curl_setopt($curl, CURLOPT_POST, true);			  		// set post
    curl_setopt($curl, CURLOPT_POSTFIELDS, $curl_post_data);	// set post fields 
    curl_setopt($curl, CURLOPT_HTTPHEADER, array ( "Accept: application/xml" ));  // set accept header
    $curl_response = curl_exec($curl);
    $headers = curl_getinfo($curl);
    curl_close($curl);
    
    if ($curl_response == true) {
      $xml = SmintTestUtils::validateXMLResponse($headers, $curl_response, $expectedHTTPStatus,  $expectedXMLElements, $expectedXMLAttributes, $this );
      
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

