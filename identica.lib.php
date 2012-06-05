<?php
/**
 * @file
 * Classes to implement the full Identica API
 */


class IdenticaUser {

  public $uid;

  public $id;

  public $screen_name;

  public $name;

  public $location;

  public $description;

  public $profile_image_url;

  public $created_at;

  public $profile_url;

  protected $oauth_token;

  protected $oauth_token_secret;

  public function __construct($values=array()) {
    $this->uid = $values['uid'];
    $this->id = $values['id'];
    $this->screen_name = $values['screen_name'];
    $this->name = $values['name'];
    $this->location = $values['location'];
    $this->description = $values['description'];
    $this->profile_image_url = $values['profile_image_url'];
    $this->created_at = $values['created_at'];
    $this->oauth_token = $values['oauth_token'];
    $this->oauth_token_secret = $values['oauth_token_secret'];
    $this->profile_url = $values['profile_url'];
 }
 
 public function get_auth() {
   return array('oauth_token' => $this->oauth_token, 'oauth_token_secret' => $this->oauth_token_secret);
 }

}
