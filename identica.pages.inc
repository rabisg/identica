<?php

/**
 * @file
 * Provides page builder functions
 */

/**
 * Form  builder for Admin Panel
 */
function identica_settings($form, &$form_state) {
  $form['identica_fedora'] = array(
    '#type' => 'textfield',
    '#title' => t('Fedora\'s Identi.ca Account'),
    '#description' => t('The Insight user name which is linked to Fedora\'s account on Identi.ca.'),
    '#default_value' => variable_get('identica_fedora', NULL),
  );  
  $form['oauth'] = array(
    '#type' => 'fieldset',
    '#title' => t('OAuth Settings'),
    '#description' => t('To enable OAuth based access for Identi.ca, you must <a href="@url">register your application</a> with identica and add the provided keys here.', array('@url' => 'http://identi.ca/settings/oauthapps/new')),
  );
  $form['oauth']['callback_url'] = array(
    '#type' => 'item',
    '#title' => t('Callback URL'),
    '#markup' => url('identica/oauth', array('absolute' => TRUE)),
  );
  $form['oauth']['identica_consumer_key'] = array(
    '#type' => 'textfield',
    '#title' => t('OAuth Consumer key'),
    '#required' => TRUE,
    '#default_value' => variable_get('identica_consumer_key', NULL),
  );
  $form['oauth']['identica_consumer_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('OAuth Consumer secret'),
    '#required' => TRUE,
    '#default_value' => variable_get('identica_consumer_secret', NULL),
  );
  return system_settings_form($form);
}

/**
 * Form validator identica_settings
 */
function identica_settings_validate($form, &$form_state) {
  if (!empty($form_state['values']['identica_fedora']) && load_identica_user(user_load_by_name($form_state['values']['identica_fedora'])->uid) == NULL)
    form_set_error('identica_fedora', t('Either no Identi.ca account is asscoiated with the username or the username does not exist.'));
}


/**
 * Callback page for OAuth Requests
 */
function identica_oauth_callback() {

  module_load_include('php', 'identica', 'twitteroauth/twitteroauth');
  $oauth_callback = url('identica/oauth', array('absolute' => TRUE));

  global $user;
  $uid = $_SESSION['identica']['uid'];
  if (!$uid)
    $uid = $user->uid;

  /* If the oauth_token is old redirect to the connect page. */
  if (!isset($_SESSION['identica']['oauth_token']) || (isset($_REQUEST['oauth_token']) && $_SESSION['identica']['oauth_token'] !== $_REQUEST['oauth_token'])) {
    drupal_goto(url('user/' . $uid . '/edit/identica', array('absolute' => TRUE)));
  }

  /* Create TwitteroAuth object with app key/secret and token key/secret from default phase */
  $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET, $_SESSION['identica']['oauth_token'], $_SESSION['identica']['oauth_token_secret']);

  /* Request access tokens from twitter */
  $access_token = $connection->getAccessToken($_REQUEST['oauth_verifier']);

  /* If HTTP response is 200 continue otherwise send to connect page to retry */
  if (200 == $connection->http_code) {

  $content = '<h1>Authentication has been succesful</h1>';
  /* Create a TwitterOauth object with consumer/user tokens. */
  $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET, $access_token['oauth_token'], $access_token['oauth_token_secret']);
  $credentials = $connection->get('account/verify_credentials');
  /* The user has been verified and the access tokens can be saved for future use */
  $account = load_identica_user($uid);
  if (!$account) { 
      //Save OAuth Token and OAuth Token Secret
      db_insert('identica_oauth')
      ->fields(array(
        'uid' => $_SESSION['identica']['uid'],
        'oauth_token' => $access_token['oauth_token'],
        'oauth_token_secret' => $access_token['oauth_token_secret'],
        'timestamp' => REQUEST_TIME,
      ))
      ->execute();

     //Save User Data into Identica User Table
     db_insert('identica_users')
      ->fields(array(
        'uid' => $_SESSION['identica']['uid'],
        'id' => $credentials->id,
        'screen_name' => $credentials->screen_name,
        'name' => $credentials->name ? $credentials->name : '',
        'description' => $credentials->description ? $credentials->description : '',
        'location' => $credentials->location ? $credentials->location : '',
        'profile_image_url' => $credentials->profile_image_url ? $credentials->profile_image_url : '',
        'created_at' => $credentials->created_at,
        'profile_url' => $credentials->statusnet_profile_url,
        ))
      ->execute();

    }
    else {
      db_update('identica_oauth')
      ->fields(array(
        'oauth_token' => $access_token['oauth_token'],
        'oauth_token_secret' => $access_token['oauth_token_secret'],
        'timestamp' => REQUEST_TIME,
      ))
      ->condition('uid', $_SESSION['identica']['uid'], '=')
      ->execute();

      db_update('identica_users')
      ->fields(array(
        'uid' => $_SESSION['identica']['uid'],
        'id' => $credentials->id,
        'screen_name' => $credentials->screen_name,
        'name' => $credentials->name ? $credentials->name : '',
        'description' => $credentials->description ? $credentials->description : '',
        'location' => $credentials->location ? $credentials->location : '',
        'profile_image_url' => $credentials->profile_image_url ? $credentials->profile_image_url : '',
        'created_at' => $credentials->created_at,
        'profile_url' => $credentials->statusnet_profile_url,
        ))
      ->condition('uid', $_SESSION['identica']['uid'], '=')
      ->execute();
    }
   $content .= t('<a href="@1">Click here to view your profile</a>', array('@1' => url('user/' . $uid . '/edit/identica', array('absolute' => TRUE))));
  }
  else {
    /* Save HTTP status for error dialog on connnect page.*/
    $content = '<h1>Authentication has not been succesful</h1>';
    $content .= l(t('Try again'), url('user/' . $uid . '/edit/identica', array('absolute' => TRUE)));
  }
  /* Remove no longer needed request tokens */
  unset($_SESSION['identica']);

  return $content;
}
