<?php
require_once 'PHPUnit/Framework.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";
 
class QueryTestAfterSmafeDist extends PHPUnit_Framework_TestCase
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
    
    public function testQueryRemoteFiles() {
      foreach ($this->testfilesremote as $key => $file) {
        $file_url = $file[0];
        $ext_key =  $file[1];
        $collection = $file[2];
        
        if ( strlen($ext_key) > 0 ) {
          # only if an external key is available 
          $this->myQueryTest("$ext_key", $collection, "200", array("query", "result"), array("collection", "external_key", "id") );
        }

      }
    }

    public function testQueryLocalFiles() {
      /*
        TODO it seems that not all files were added by smafewrap -> check smafewrap an revisit this test
      */
      
      // workaround for proplem: fixed list of sucessfully added tracks: 
      $this->testfileslocal = array( 
        array("","url1"                 ,""),
        array("","url1-C"               ,"C"),
        array("","url2"                 ,""),
        array("","url2-C"               ,"C"),
        array("","url3"                 ,""),
        array("","url3-C"               ,"C"),
        array("","url4"                 ,""),
        array("","url4-C"               ,"C"),
        array("","mykey2-A"             ,"A"),
        array("","mykey3-A"             ,"A"),
        array("","mykey4-A"             ,"A"),
        array("","mykey5-A"             ,"A"),
        array("","myotherotherkey1-d"   ,""),
        array("","myotherkey2-A"        ,"A"),
        array("","myotherkey3-A"        ,"A"),
        array("","myotherkey4-A"        ,"A"),
        array("","myotherkey5-A"        ,"A"),
      );
      
      foreach ($this->testfileslocal as $key => $file) {
        $file_url = $file[0];
        $ext_key =  $file[1];
        $collection = $file[2];
        
        if ( strlen($ext_key) > 0 ) {
          # only if an external key is available 
          $this->myQueryTest("$ext_key", $collection, "200", array("query", "result"), array("collection", "external_key", "id") );
        }

      }
    }
    
    
    public function testQueryWithnonexistingCollection()
    {
      foreach ($this->testfilesremote as $key => $file) {
        $file_url = $file[0];
        $ext_key =  $file[1];
        $collection = $file[2];
        
        if ( strlen($ext_key) > 0 ) {
          # only if an external key is available 
          $this->myQueryTest("$ext_key", "foobar", "404", array("error", "errorDetail"), array("code") );
        }

      }
    }

    
    private function myQueryTest($trackextkey, $collectionname=null, $expectedHTTPStatus="200", $expectedXMLElements, $expectedXMLAttributes ){

      $config = TestConfig::getConfig();
      $service_url = $config['server']['baseurl'].'/track_external_key/';
      
      $url = $service_url . $trackextkey ; 
      
      if ( !is_null($collectionname) && (strlen($collectionname)>0) ) {
         $url =  $url . "?collection=" . urlencode($collectionname);
      }
      
      
      $curl = curl_init($url);
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);  
      curl_setopt($curl, CURLOPT_HTTPHEADER, array ( "Accept: application/xml" ));  
      $curl_response = curl_exec($curl);
      $headers = curl_getinfo($curl);
      curl_close($curl);
      
      if ($curl_response == true) {
        $xml = SmintTestUtils::validateXMLResponse($headers, $curl_response, $expectedHTTPStatus, $expectedXMLElements, $expectedXMLAttributes, $this );

        MyLog::printWithDuration( "Expected: $expectedHTTPStatus - queried with the result: ". str_replace("\n", "", $curl_response) ); 
        
      } else {
        echo __METHOD__ . "\n";
        echo "curl_response:".$curl_response. "\n";      
        print_r($headers);
        $this->fail("There was a problem accessing the api! \n\n" );
      }
      
    }
}

?>