<?php
require_once 'PHPUnit/Framework.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";
 
class ExternalKeyTest extends PHPUnit_Framework_TestCase
{
    public function testEmptyQuery()
    {
      $this->myQueryTest("");
    }

    public function testNonExistingTrackQuery()
    {
      $this->myQueryTest("foobar");
    }
    
    private function myQueryTest($trackid){
      $config = TestConfig::getConfig();
      $service_url = $config['server']['baseurl'].'/track_external_key/';
      
      $curl = curl_init($service_url.$trackid);
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);  
      curl_setopt($curl, CURLOPT_HTTPHEADER, array ( "Accept: application/xml" ));  
      $curl_response = curl_exec($curl);
      $headers = curl_getinfo($curl);
      curl_close($curl);
      
      if ($curl_response == true) {
        $xml = SmintTestUtils::validateXMLResponse($headers, $curl_response, "404", array("error", "errorDetail"), array("code"), $this );
      } else {
        echo __METHOD__ . "\n";
        echo "curl_response:".$curl_response. "\n";      
        print_r($headers);
        $this->fail("There was a problem accessing the api! \n\n" );
      }
      
    }
}

?>