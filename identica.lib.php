<?php
/**
 * @file
 * Classes to implement the full Identica API
 */


class IdenticaUser {

  public $id;

  public $screen_name;

  public $name;

  public $location;

  public $description;

  public $url;

  public $profile_image_url;

  public $created_at;

  public $created_time;

  protected $oauth_token;

  protected $oauth_token_secret;

  public function __construct($values = array()) {
    $this->id = $values['id'];
    $this->screen_name = $values['screen_name'];
    $this->name = $values['name'];
    $this->location = $values['location'];
    $this->description = $values['description'];
    $this->url = $values['url'];
    $this->created_at = $values['created_at'];
    if ($values['created_at'] && $created_time = strtotime($values['created_at'])) {
      $this->created_time = $created_time;
    }
    $this->utc_offset = $values['utc_offset']?$values['utc_offset']:0;
    $this->oauth_token = $values['oauth_token'];
    $this->oauth_token_secret = $values['oauth_token_secret'];
  }

  public function get_auth() {
    return array('oauth_token' => $this->oauth_token, 'oauth_token_secret' => $this->oauth_token_secret);
  }

}
