<?php
require_once 'PHPUnit/Framework.php';
require_once dirname(__FILE__). "/../helpers/bootstrapservertest.php";
 
class WrongContentTypeTest extends PHPUnit_Framework_TestCase
{
    
    public function testSimple()
    {
      $config = TestConfig::getConfig();
      $service_url = $config['server']['baseurl'].'/version';
      
      $curl = curl_init($service_url);
      curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);  
      curl_setopt($curl, CURLOPT_HTTPHEADER, array ( "Accept: application/xmlx" ));  
      $curl_response = curl_exec($curl);
      $headers = curl_getinfo($curl);
      curl_close($curl);
      
    
      $xml = SmintTestUtils::validateXMLResponse($headers, $curl_response, "406", array("error"), array("code"), $this );


      
      $errorAttributes = array();
       foreach($xml->error[0]->attributes() as $name => $value) {
           $versionAttributes[$name] = $value;
           // test if all values are numbers 
           $this->assertType(PHPUnit_Framework_Constraint_IsType::TYPE_NUMERIC , (string)$value, "Type of ErrorCode is wrong." );
       }
 
      
      
    }
    
}

?>