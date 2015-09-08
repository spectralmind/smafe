<?php 
/**
 * Tools needed by the API
 */

class SmafeApiUtils
{
  public static function getUUID() {
    $uuid = uniqid(rand(),true); 
    return $uuid;
  }
  


}