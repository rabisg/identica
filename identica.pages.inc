<?php

/**
 * @file
 * Provides page builder functions
 */

define('CONSUMER_KEY', variable_get('identica_consumer_key', ''));
define('CONSUMER_SECRET', variable_get('identica_consumer_secret', ''));
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
  if(!empty($form_state['values']['identica_fedora']) && _get_identica_account($form_state['values']['identica_fedora'])==NULL)
    form_set_error('identica_fedora', t('Either no Identi.ca account is asscoiated with the username or the username does not exist.'));
}

/**
 * Page builder function to allow users to add Identi.ca accounts
 */
function identica_user_settings($uid) {
  global $user;
  if(!$uid)
    $uid = $user->uid;
  if(($account = _get_identica_account($uid))!=NULL) {
    // Display the current account details
    // $output['details'] = 
    $output['form'] = drupal_get_form('identica_add_account_form' , $uid, TRUE);
  }
  else {
    $output['form'] = drupal_get_form('identica_add_account_form' , $uid, FALSE);
  }
  return $output;
}

/**
 * Form  builder for Adding/Removing Identi.ca accounts
 * @params
 * uid The User Id of the person whose Identi.ca account is added
 * accountExists Denotes whether an account is already associated with the user or not
 *               If yes, then adds a Remove Button accordingly
 */
function identica_add_account_form($form, &$form_state, $user, $accountExists=FALSE) {
  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $user->uid,
  );
  $form['add_account'] = array(
    '#type' => 'submit',
    '#value' => $accountExists ? t('Change Existing Account') : t('Add New Account'),
  );
  if($accountExists) {
    $form['remove_account'] = array(
      '#type' => 'submit',
      '#value' => t('Remove Account'),
      '#submit' => array('identica_remove_account_submit'),
    );
  }
  return $form;
}

/**
 * Implements FORM_ID_validate
 * Intercept submission of 'Add Account' form on
 * user/%/edit/identica page and redirect to Identica for auth. 
 */
function identica_add_account_form_validate($form, &$form_state) {
  if (CONSUMER_KEY == '' || CONSUMER_SECRET == '') {
    form_set_error('', t('Please configure Identica consumer key and secret.'));
  }

  module_load_include('php', 'identica', 'twitteroauth/twitteroauth');
  $oauth_callback = url('identica/oauth', array('absolute' => TRUE));
  /* Build TwitterOAuth object with client credentials. */
  $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET);
 
  /* Get temporary credentials. */
  $request_token = $connection->getRequestToken($oauth_callback);
  $token = $request_token['oauth_token'];
  
  /* Save temporary credentials to session. */
  $_SESSION['identica']['oauth_token'] = $token = $request_token['oauth_token'];
  $_SESSION['identica']['oauth_token_secret'] = $request_token['oauth_token_secret'];
  $_SESSION['identica']['uid'] = $form_state['values']['uid'];
 
  /* If last connection failed don't display authorization link. */
  switch ($connection->http_code) {
    case 200:
      /* Build authorize URL and redirect user to Identi.ca. */
      $url = $connection->getAuthorizeURL($token);
      drupal_goto($url); 
      break;
    default:
      /* Show notification if something went wrong. */
      form_set_error('', t('Could not connect to Identi.ca. Refresh the page or try again later.'));
  }
}

/**
 * Submit function for removing accounts
 */
function identica_remove_account_submit($form, &$form_state) {  
  $uid = $form_state['values']['uid'];
  if(_get_identica_account($uid)!=NULL) {
    ;
    //delete entry
  }
}

/**
 * Callback page for OAuth Requests
 */
function identica_oauth_callback() {

  module_load_include('php', 'identica', 'twitteroauth/twitteroauth');
  $oauth_callback = url('identica/oauth', array('absolute' => TRUE));

  global $user;
  $uid = $_SESSION['identica']['uid'];
  if(!$uid)
    $uid = $user->uid;

  /* If the oauth_token is old redirect to the connect page. */
  if (!isset($_SESSION['identica']['oauth_token']) || (isset($_REQUEST['oauth_token']) && $_SESSION['identica']['oauth_token'] !== $_REQUEST['oauth_token'])) {
    drupal_goto(url('user/' . $uid . '/edit/identica', array('absolute' => TRUE)));
  }

  /* Create TwitteroAuth object with app key/secret and token key/secret from default phase */
  $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET, $_SESSION['identica']['oauth_token'], $_SESSION['identica']['oauth_token_secret']);

  /* Request access tokens from twitter */
  $access_token = $connection->getAccessToken($_REQUEST['oauth_verifier']);

 
  /* Remove no longer needed request tokens */
  //unset($_SESSION['identica']['oauth_token']);
  //unset($_SESSION['identica']['oauth_token_secret']);

  /* If HTTP response is 200 continue otherwise send to connect page to retry */
  if (200 == $connection->http_code) {
    /* The user has been verified and the access tokens can be saved for future use */
    $content = '<h2>Authentication has been succesful</h2>';
    /* Create a TwitterOauth object with consumer/user tokens. */
    $connection = new TwitterOAuth(CONSUMER_KEY, CONSUMER_SECRET, $access_token['oauth_token'], $access_token['oauth_token_secret']);
    /* If method is set change API call made. Test is called by default. */
    $content .= $connection->get('account/verify_credentials');
  } else {
    /* Save HTTP status for error dialog on connnect page.*/
    $content = '<h2>Authentication has not been succesful</h2>';
  }
  return $content;
}
