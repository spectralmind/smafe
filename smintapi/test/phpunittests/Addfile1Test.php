<?php
require_once 'PHPUnit/Framework.php';
require_once 'PHPUnit/Extensions/Database/TestCase.php';
require_once 'PHPUnit/Extensions/Database/DataSet/DefaultDataSet.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";
 
class Addfile1Test extends PHPUnit_Extensions_Database_TestCase
{
	
	 protected function getConnection()
    {
    	$testconfig = TestConfig::getConfig();
		
		$dbname = $testconfig["database"]["dbname"];
	      $dbhost = $testconfig["database"]["dbhost"]; 
	      $dbport = $testconfig["database"]["dbport"]; 
	      $dbuser = $testconfig["database"]["dbuser"];
	      $dbpassword = $testconfig["database"]["dbpassword"]; 
     
        $pdo = new PDO("pgsql:dbname=$dbname;host=$dbhost;port=$dbport", $dbuser, $dbpassword, array(PDO::ATTR_PERSISTENT => true));
		
        return $this->createDefaultDBConnection($pdo, $dbname);
    }
 
    protected function getDataSet()
    {
    	 return new PHPUnit_Extensions_Database_DataSet_DefaultDataSet();
        //return $this->createFlatXMLDataSet(dirname(__FILE__).'/../resources/Addfile1Test.xml');
    }
	
	
	
	
	/**
   * adds all files defined in the array testfilesremote
   *
   * @dataProvider provider_testLoadFileFromURL
   * @return void
   * @author jochum
   */
  public function testLoadFileFromURL($file_url, $ext_key, $collection)
  {
  	
       MyLog::printWithDuration( "Adding URL:$file_url Ext.Key:$ext_key Collection:$collection"); 
      
      $expectedElements = array("track_added"); 
      $expectedAttributes = array( "url" ,"smint_track_id" ); 
      if ( is_string($ext_key) && ( strlen($ext_key) > 0 ) ) 
        $expectedAttributes[] = "external_key"; 

      $this->myFileAddTest($file_url, $ext_key, $collection, 200, $expectedElements, $expectedAttributes);
  }
  
  
  public function provider_testLoadFileFromURL()
    {
        return array(
          array('file_url' => "r1",
		  	'ext_key' => 'extkey',
		  	'collection' => 'A'
		  )
        );
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
	  echo "headers:\n";
      print_r($headers);
      echo "curl_post_data:\n";
      print_r($curl_post_data);      
      $this->fail("There was a problem accessing the api! \n\n" );
    }
    
  }
	
}

?>