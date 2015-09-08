<?php
require_once 'PHPUnit/Framework.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";
 
class VersionTest extends PHPUnit_Framework_TestCase
{
    public function testSimple()
    {
      $config = TestConfig::getConfig();
      $service_url = $config['server']['baseurl'].'/version';
      
      $curl = curl_init($service_url);
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);  
      curl_setopt($curl, CURLOPT_HTTPHEADER, array ( "Accept: application/xml" ));  
      $curl_response = curl_exec($curl);
      $headers = curl_getinfo($curl);
      curl_close($curl);
      
      // check if http code fits 
      if ( $headers["http_code"] != 200 )  $this->fail("Response has the wrong http code : " . $headers["http_code"] ."\n\n" . print_r($headers,true) . "\n\n" . $curl_response );
      
      
      // parse xml result 
      try {
        $xml = new SimpleXMLElement($curl_response);
      } catch (Exception $e) {
        $this->fail("An exception has been raised: " . $e->getMessage() ."\n\n" . $e->getTraceAsString() );
      }

      $versionAttributes = array();
      foreach($xml->version[0]->attributes() as $name => $value) {
          $versionAttributes[$name] = $value;
// test if all values are numbers 
          $this->assertType(PHPUnit_Framework_Constraint_IsType::TYPE_NUMERIC , (string)$value, "Type of Attribute is wrong." );
      }
      
      $mandatoryAttributes = array ("major", "minor", "revision"); 
      foreach ($mandatoryAttributes as $key => $value) {
// test if all attributes are included 
        $this->assertArrayHasKey( $value, $versionAttributes, "Attribute missing in result: \n\n" . $curl_response);
      }
      
    }
}

?>