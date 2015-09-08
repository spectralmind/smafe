<?php
require_once 'PHPUnit/Framework.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";
 
class SimpleQueryTest extends PHPUnit_Framework_TestCase
{
    
    protected function setUp()
    {
      // place setup here
    }

    protected function tearDown()
    {
      // place teardown here
    }
    
    
    public function testQueryWithoutCollection()
    {
      $this->myQueryTest("testAddUrlKey_1", null, "200", array("query", "result"), array("id", "external_key") );
    }

    public function testQueryWithnonexistingCollection()
    {
      $this->myQueryTest("testAddUrlKey_1", "foobar", "404", array("error", "errorDetail"), array("code") );
    }

    public function testQueryWithexistingCollection()
    {
      $this->myQueryTest("testAddUrlKey_2", "_d", "200", array("query", "result"), array("id", "external_key") );
    }

    public function testQueryWithexistingCollection2()
    {
      $this->myQueryTest("testAddUrlKey_withCollectionName", "testCollection", "200", array("query", "result"), array("id", "external_key") );
    }

    
    private function myQueryTest($trackextkey, $collectionname=null, $expectedHTTPStatus="200", $expectedXMLElements, $expectedXMLAttributes ){

      $config = TestConfig::getConfig();
      $service_url = $config['server']['baseurl'].'/track_external_key/';
      
      $url = $service_url . $trackextkey ; 
      
      if ( !is_null($collectionname) ) {
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
      } else {
        echo __METHOD__ . "\n";
        echo "curl_response:".$curl_response. "\n";      
        print_r($headers);
        $this->fail("There was a problem accessing the api! \n\n" );
      }
      
    }
}

?>