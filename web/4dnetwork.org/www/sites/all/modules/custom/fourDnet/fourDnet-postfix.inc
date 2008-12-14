<?php


/** Get the email database.table
 */
function _fourDnet_email_table(){
  return 'mailsql.users';
}

/** Get the email domain
 */
function _fourDnet_email_domain(){
  return '4dnetwork.org';
}

/** Get the email root directory
 */
function _fourDnet_email_home_path(){
  return '/home/vmail';
}

/** Install the postfix bridge
 */
function _fourDnet_install_postfix(){
  $fields = db_query("DESC ". _fourDnet_email_table() );
  while ($field = db_fetch_object($fields)) {
    $field_array[] = $field->Field;
    }
  if (!in_array('drupal_uid', $field_array)){
   db_query('ALTER TABLE '. _fourDnet_email_table() . ' ADD drupal_uid INT(10) unsigned NOT NULL default \'0\' AFTER id');
   return true;
   }
   return false;
}


function _fourDnet_insert_postfix_account(&$account){
//   fourDnet_debug('insert_postfix - $account: ' . print_r($account, true) );
  
  // Assemble the user info...
  $emailUser = _fourDnet_make_user_name($account);
  $emailHost = _fourDnet_email_domain();
  $homeDir = _fourDnet_email_home_path();
  if( $emailUser && $emailHost && $homeDir ){
   $emailAddress = $emailUser .'@'. $emailHost;
   $mailDir = $emailHost.'/'.$emailUser;
   $displayName = $account->profile_first_name . " " . $account->profile_surname;
   
   $post_query = "INSERT INTO "._fourDnet_email_table()." " .
      "(id, drupal_uid, username, email, password, name, uid, gid, homedir, maildir, quota, postfix) VALUES " .
      "('NULL', '%d', '%s', '%s', '%s', '%s', '5000', '5000', '%s', '%s', '0', 'y')";
   db_query($post_query, $account->uid, $emailUser, $emailAddress, $account->pass, $displayName, $homeDir, $mailDir );
   }
  return;
}




function _fourDnet_update_postfix_account(&$edit, &$account, $category){
  // fourDnet_debug('update_postfix - $edit: ' . print_r($edit, true) );
  
  /* If we're editing the user 'account', update the user password in the maildb */
  if( $category == 'account' ){
   $mail_query = "UPDATE "._fourDnet_email_table()." SET password = '%s' WHERE drupal_uid = %d";
   db_query($mail_query, $account->pass, $account->uid);
   }
  
  /* If we're editing the user 'Contact Details', update the users display name
   * in the maildb - But don't update the email.address ???
   */
  elseif( $category == 'Contact Details' ){
   $displayName = $edit['profile_first_name'] . " " . $edit['profile_surname'];
   $mail_query = "UPDATE "._fourDnet_email_table()." SET name = '%s' WHERE drupal_uid = %d";
   db_query($mail_query, $displayName, $account->uid);
   }
  return;
}



function fourDnet_get_postfix_address($uid){
  $email = db_fetch_object(db_query("SELECT email FROM "._fourDnet_email_table()." WHERE drupal_uid = '%d'", $uid));
  if( isset($email->email) && $email->email ) {
   return $email->email;
   }
  return;
}

function _fourDnet_load_postfix_account(&$account, $force=false){
//   fourDnet_debug('load_postfix - $account: ' . print_r($account, true) );
  
  // Skip if already loaded...
  if($force || !isset($account->postfix) ){
   $email = db_fetch_object(db_query("SELECT id,email,homedir,maildir,quota FROM "._fourDnet_email_table()." WHERE drupal_uid = '%d'", $account->uid));
   if($email->id) {
    $account->postfix = array(
                  'id' => $email->id,
                  'email' => $email->email,
                  'homedir' => $email->homedir,
                  'maildir' => $email->maildir,
                  'quota' => $email->quota
                 );
    }
   }
  return $account->postfix;
}



function _fourDnet_delete_postfix_account(&$account){
//   fourDnet_debug('delete_postfix - $account: ' . print_r($account, true) );
  
  // TODO: Delete the users email??
  // - For now just flag the account inactive;
  
  // $mail_query = "UPDATE "._fourDnet_email_table()." SET active = '0' WHERE drupal_uid = %d"; // theres no 'active field in the mail table (yet?)
  $mail_query = "UPDATE "._fourDnet_email_table()." SET quota = '-1' WHERE drupal_uid = %d";
  db_query($mail_query, $account->uid);
  return;
}



