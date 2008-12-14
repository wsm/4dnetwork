<?php


/** Make a first.lastname out of the Users Firstname and Surname
 */
function _fourDnet_make_user_name(&$account){
  // fourDnet_debug('make_user_name - $account: ' . print_r($account, true) );
  if (is_array($account)) {
   $first = $account['profile_first_name'];
   $last = $account['profile_surname'];
   }
  else{
   $first = $account->profile_first_name;
   $last = $account->profile_surname;
   }
  
  $name = '';
  if( $first ){
   $name = strtolower(preg_replace('/[^a-zA-Z0-9.\-]/is', '', $first));
   }
  if( $last ){
   $name .= ($first) ? '.' : '';
   $name .= strtolower(preg_replace('/[^a-zA-Z0-9.\-]/is', '', $last));
   }
  
  return ($name) ? $name : false;
}


/** We define _fourDnet_user_register_validate callback in the registration
 * form.  This provides an opportunity to override the users registered
 * "Username" (Hint: Username is hidden on the registration form).
 * This is to enforce the 'firstname.lastname' convention for user names.
 */
function _fourDnet_user_register_validate(&$form, $form_id, &$formx) {
//   fourDnet_debug('user_register_validate: ' . $form_id );
  $ref = array( 'profile_first_name'=>$formx['#post']['profile_first_name'], 'profile_surname'=>$formx['#post']['profile_surname'] );
  $name = _fourDnet_make_user_name( $ref );
  form_set_value($form, $name );
}

/**
 * Implementation of hook_user()
 */
function fourDnet_user($op, &$edit, &$account, $category = NULL) {
 // fourDnet_debug(' _user: '. $op. '; $category: '.$category);
 // drupal_set_message(' _user: $op: '.$op.'; $edit: '. print_r($edit, true) . '; $account: '.print_r($account, true) );

  if( $op == 'insert' ) {
   /* Nasty hack here...
    * Weve hidden the user name input element on the form to enforce
    * registration under the 'firstname.lastname' convention, but
    * unfortunately there is no clear way to override the username
    * before it gets entered in the database.
    * So this hack updates the newusers login with their
    * first.lastname then insert the email record for them as well.
    */
   $user = user_load( array('uid'=>$account->uid) );
//    /* UPDATE: Worked-around this ugly hack; @see: _fourDnet_user_register_validate() */
//    $user->name = _fourDnet_make_user_name( $user );
//    $user_query = "UPDATE {users} SET name = '%s' WHERE uid = %d";
//    db_query($user_query, $user->name, $user->uid);
   return _fourDnet_insert_postfix_account($user);
   }
  elseif( $op == 'update' && ($category == 'account' || $category == 'Contact Details') ) {
   $acct = user_load( array('uid'=>$account->uid) );
   $pfix = _fourDnet_load_postfix_account($acct, true);
   if( !is_array($pfix) || !count($pfix) ){
    _fourDnet_insert_postfix_account($acct);
    }
   /*  Password has not been updated by core yet - manually generated md5sum */
   $acct->pass = md5($edit['pass']);
   return _fourDnet_update_postfix_account($edit, $acct, $category);
   }
  elseif( $op == 'load' ) {
   //_fourDnet_load_postfix_account($account);
   
   //fourDnet_debug(' _user: $op: '.$op.'; $edit: '. print_r($edit, true) . '; $account: '.print_r($account, true) );
   if( !isset($account->profile_email) || !$account->profile_email ){
    $account->profile_email = $account->mail;
    }
   }
  elseif( $op == 'view' ) {
   
   _fourDnet_load_postfix_account($account);
   if( isset($account->postfix) ){
    $info['4D Notesharing']['postfix'] = array('title'=>'Email address', 'value'=>$account->postfix['email'], 'class'=>'some-class' );
    return $info;
    }
   }

//   elseif( $op == 'register' ) { // New Registration. form
//    /* UPDATE: Changed - using hook_form_alter now; it's a little cleaner?
//    return _fourDnet_user_registration_form();
//    }

  elseif($op == 'form' && $category == 'account' ) {     // Edit User form
   // return _fourDnet_load_postfix_account($account);
   }
  elseif( $op == 'delete' ) {
   return _fourDnet_delete_postfix_account($account);
   }

  return;
}



/*
 * Internal _fourDnet_user_registration_form_alter()
 */
function _fourDnet_user_registration_form_alter($form_id, &$form) {
  // fourDnet_debug('alter: ' . $form_id . ' frm: '. print_r($form, true) );
  $form['account'] = array('#type' => 'fieldset',
    '#title' => t('Account information'),
    '#weight' => 10,
  );
  $form['account']['name'] = array('#type' => 'hidden',
     '#default_value' => 'newuser',
     '#required' => TRUE,
     '#validate' => array('_fourDnet_user_register_validate' => array($form_id, $form))
      );
  if (!variable_get('user_email_verification', TRUE) || $admin) {
   $form['account']['pass'] = array(
      '#type' => 'password_confirm',
      '#description' => t('Provide a password for the new account in both fields.'),
      '#required' => TRUE,
      '#size' => 25,
    );
   }
  $form['account']['mail'] = array('#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#default_value' => $edit['mail'],
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#description' => t('A valid e-mail address. All e-mails from the system will be sent to this address. The e-mail address is not made public and will only be used if you wish to receive a new password or wish to receive certain news or notifications by e-mail.'),
    '#required' => TRUE,
  );
}




