<?php

/**
 * Implementation of hook_perm().
 * @return array
 */
function taxonomy_node_perm() {
  return array(
          'access taxonomy nodes',
          'create taxonomy nodes',
          'create personal taxonomy nodes',
          'create private taxonomy nodes',
          'edit taxonomy nodes',
          'edit own taxonomy nodes',
          );
}


/**
 * Implementation of hook_node_info().
 */
function taxonomy_node_node_info() {
  return array(
    'term_node' => array(
      'name'        => t('Term Node'),
      'module'      => 'taxonomy_node_term_node',
      'description' => t('Create a new Term node.'),
      'help'        => t('Create a new Term node.'),
      'body_label'  => t('Description'),
    ),
  );
}


/**
 *  Implementation of hook_access().
 */
function taxonomy_node_term_node_access($op, $node) {
  switch($op) {
  	case 'view':
  	  return user_access('access taxonomy nodes');
  	  break;
  	case 'create':
  	  return user_access('create taxonomy nodes');
  	  break;
  	case 'update':
  	case 'delete':
  	  global $user;
  	  return (
                 user_access('edit taxonomy nodes') ||
                 (user_access('edit own taxonomy nodes') && $node->uid == $user->uid )
                 );
  	  break;
  }
}


/**
 *  Implementation of hook_access() - Extra Privacy control
 */
function _taxonomy_node_term_node_access_node_privacy($tid) {
  //drupal_set_message('taxonomy_node_term_node_access(); $tid: '.print_r($tid, true) );

  $tp = db_fetch_object(db_query('SELECT uid, private AS is_private FROM {taxonomy_node} WHERE term_id = %d', $tid));
  global $user;
  return ( !$tp->is_private || $tp->uid == $user->uid );
}



/**
 * Implementation of hook_form().
 */
function taxonomy_node_term_node_form(&$node, &$param) {
  
  $vocabs = _taxonomy_node_get_enabled_vocabs();
  
  if( !$vocabs ){
   drupal_set_message('Warning: Taxonomy Node Vocabularies are not configured.', 'error');
   return;
   }
  
  $title = '';
  $excludes = array();
  
  /* --- Edit existing node --- */
  if( $node->nid ){
   $term = _taxonomy_node_get_term_for_node($node);
   if( !$term ){
    drupal_set_message('Associated Term Not Found.', 'error');
    return;
    }
   $title = $node->title;
   $onlyVocab = $term->vid;
   $parents = array_keys( taxonomy_get_parents($term->tid) );
   if( empty($parents) ) $parents = -1;
   $excludes[] = $term->tid;
   }
  
   /* --- Add a new node to an already existing term --- */
   elseif( 'term' ==  arg(3) ){
    if( is_numeric(arg(4)) ){
     $term = taxonomy_get_term(arg(4));
     }
    if( !$term ){
     drupal_set_message('Invalid Term', 'error');
     return;
     }
    $title = $term->name;
    $onlyVocab = $term->vid;
    $parents = array_keys( taxonomy_get_parents($term->tid) );
    //taxonomy_node_debug('term_node_form: $parents: '.print_r($parents) );
    if( empty($parents) ) $parents = -1;
    $excludes[] = $term->tid;
    
    $form['taxonomy_node_add_term'] = array(
      '#type' => 'hidden',
      '#required' => TRUE,
      '#default_value' => $term->tid,
      );
    }
  
  /* --- Add new term & node --- */
  else{
   /* This switch could allow multiple vocabs, but would cause issues
    * in hook_delete() and hook_insert() - so it's not allowed, for now.
    *
    * $onlyVocab = ( is_numeric(arg(3)) ) ? arg(3) : 0;
    */
   $onlyVocab = ( is_numeric(arg(3)) ) ? arg(3) : $vocabs[0];
   $parents = ( is_numeric(arg(4)) ) ? arg(4) : -1;
   }
  
  
  $form['taxonomy_parent']['#tree'] = TRUE;
  foreach( $vocabs as $i=>$vid ){
    $vocab = taxonomy_get_vocabulary($vid);
//     if (!$vocab->multiple) {
//      $blank = ($vocab->required) ? t('- Please choose -') : t('- None selected -');
//      }
//     else {
//      $blank = ($vocab->required) ? 0 : t('- None -');
//      }
     $blank = '';
    
    if( $onlyVocab ){
     if( $vid == $onlyVocab ){
      $form['taxonomy_parent'][$vid] = _taxonomy_term_select(check_plain($vocab->name), 'taxonomy', $parents, $vid, NULL, intval($vocab->multiple), $blank, $excludes );


//      // Pre-select the taxonomy term if provided...
//      if( is_numeric(arg(4)) ){
//       $vid = _fourDnet_get_project_tree_vocabs();
//       $term = taxonomy_get_term(arg(4));
//          if( $term->vid == $vid ) {
//           //taxonomy_node_debug('form_alter; $possibility: '. print_r($term, TRUE) );
//           $taxo = new stdClass();
//           $taxo->tid = $term->tid;
//           $taxo->vid = $term->vid;
//           $taxo->name = $term->name;
//           $taxo->description = $term->description;
//           $node->taxonomy[$term->tid] = $taxo;
//          }
//       //taxonomy_node_debug('form_alter; $node: ' . print_r($node, TRUE) );
//       }

      }
     }
    else{
     $form['taxonomy_parent'][$vid] = _taxonomy_term_select(check_plain($vocab->name), 'taxonomy', $parents, $vid, NULL, intval($vocab->multiple), $blank, $excludes );
     }

    if( isset($form['taxonomy_parent'][$vid]) ){
     $choice = new stdClass();
     $choice->option = array('0' => t('--- Top Level ---'));
     array_unshift($form['taxonomy_parent'][$vid]['#options'], $choice );
     $form['taxonomy_parent'][$vid]['#required'] = TRUE;
     $form['taxonomy_parent'][$vid]['#title'] = check_plain($v->name) . ' Parent';
     }
    }
  
  // We need to define form elements for the node's title and body.
  $type = node_get_types('type', $node);
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => TRUE,
    '#default_value' => $title,
    '#weight' => -5
  );
  // We want the body and filter elements to be adjacent; stuff them in a sub-array.
  $form['body_filter']['body'] = array(
    '#type' => 'textarea',
    '#title' => check_plain($type->body_label),
    '#default_value' => $node->body,
    '#required' => FALSE,
    '#rows' => 20
  );
  $form['body_filter']['filter'] = filter_form($node->format);
  
  if( user_access('create personal taxonomy nodes') ){
   $form['is_personal'] = array(
     '#type' => 'checkbox',
     '#title' => 'Personal',
     '#required' => FALSE,
     '#default_value' => $node->taxonomy_node_is_personal,
     '#description' => 'If set will be considered "Personal" rather than "Community".',
     //'#weight' => -5
     );
  
   if( user_access('create private taxonomy nodes') ){
    $form['is_private'] = array(
      '#type' => 'checkbox',
      '#title' => 'Privacy Control',
      '#required' => FALSE,
      '#default_value' => $node->taxonomy_node_is_private,
      '#description' => 'If set will only be accessible by yourself.  Must also be set "Personal" above.',
      //'#weight' => -5
      );
    }
   }
  
  return $form;
}



/**
 * Implementation of hook_delete().
 */
function taxonomy_node_term_node_delete($node) {
  db_query("DELETE FROM {taxonomy_node} WHERE node_id = %d", $node->nid);
  taxonomy_del_term( $node->taxonomy_node_term_id );
}

/**
 * Implementation of hook_load().
 */
function taxonomy_node_term_node_load($node) {
  if( _taxonomy_node_is_content_type($node->type) ){
   $add = db_fetch_object(db_query("SELECT term_id AS taxonomy_node_term_id, personal AS taxonomy_node_is_personal, private AS taxonomy_node_is_private FROM {taxonomy_node} WHERE node_id = %d", $node->nid));
   return $add;
   }
}



/**
 * Implementation of hook_validate().
 */
function taxonomy_node_term_node_validate($node, &$form) {
  if( !$node->taxonomy_parent || !is_array($node->taxonomy_parent) ){
   form_set_error('taxonomy_parent', 'Please select a Parent');
   }
}

/**
 * Implementation of hook_submit().
 */
function taxonomy_node_term_node_submit(&$node) {
  _taxonomy_node_term_node_save('submit', $node);
}

/**
 * Implementation of hook_insert().
 */
function taxonomy_node_term_node_insert($node) {
  _taxonomy_node_term_node_save('insert', $node);
}


/**
 * Save the Term Node.
 */
function _taxonomy_node_term_node_save($hook, &$node) {

  // Add new terms under the vocabularies...
  foreach($node->taxonomy_parent as $vid=>$parent){
    $parentids = array();
    
    // A new term may have multiple parents...
    if( !is_array($parent) ){
     $parentids[] = $parent;
     }
    else{
     foreach($parent as $i=>$p){
       $parentids[] = $p;
       }
     }
    
    $newTerm = array(
               'vid' => $vid,
               'parent' => $parentids,
               'name' => $node->title
               );
    
    /*  Enforce Personal/Private  */
    if( !user_access('create personal taxonomy nodes') ){
     $node->is_personal = 0;
     $node->is_private = 0;
     }
    if( !user_access('create private taxonomy nodes') ){
     $node->is_private = 0;
     }
    if( $node->is_private == 1 && $node->is_personal != 1 ){
     $node->is_private = 0;
     }
    
    /* --- Adding new Term Node --- */
    if( $hook == 'insert' ){
     // Adding a node for a term that already existed...
     if( $node->taxonomy_node_add_term ){
      $newTerm['tid'] = $node->taxonomy_node_add_term;
      }
     taxonomy_save_term($newTerm);
     
     $sql = "INSERT INTO {taxonomy_node} (id,uid,term_id,node_id,personal,private) VALUES ('NULL', '%d', '%d', '%d', '%d', '%d')";
     db_query($sql, $node->uid, $newTerm['tid'], $node->nid, $node->is_personal, $node->is_private );
     
     $_REQUEST['destination'] = 'taxonomy/term/' . $newTerm['tid'];
     }

    /* --- Editing existing Term Node --- */
    elseif( $hook == 'submit' && $node->nid ){
      $term = _taxonomy_node_get_term_for_node($node);
      $newTerm['tid'] = $term->tid;
      taxonomy_save_term($newTerm);
      
      $sql = "UPDATE {taxonomy_node} SET personal = '%d', private = '%d' WHERE node_id = '%d'";
      db_query($sql, $node->is_personal, $node->is_private, $node->nid );
      
      $_REQUEST['destination'] = 'taxonomy/term/' . $newTerm['tid'];
      }
    }
}


/**
 * Implementation of hook_link().
 */
function taxonomy_node_link($type, $node = NULL, $teaser = FALSE) {
  
  if( $type == 'node' && _taxonomy_node_is_content_type($node->type) ) {
   if( isset($node->taxonomy_node_term_id) ) {
    global $user;
    if( user_access('edit taxonomy nodes') ||
        (user_access('edit own taxonomy nodes') && $node->uid == $user->uid )
      ) {
     $links['taxonomy_node_edit'] = array(
      'title' => t('Edit').' '.check_plain($node->title),
      'href' => 'node/'.$node->nid.'/edit',
      'attributes' => array('title' => t('Edit').' '.check_plain($node->title) ),
      );
     }
    }
   }
  
  return $links;
}

 /**
  * Handle the output of the Term Nodes (override taxonomy/term/x display)
  */
function _taxonomy_node_view_term_node($tid, $op='page'){
  //drupal_set_message('_taxonomy_node_view_term_node: $tid: '.$tid );
  
  if( !is_numeric($tid) ){
   return drupal_not_found();
   }
  
  $term = taxonomy_get_term($tid);
  if( !$term ) {
   return drupal_not_found();
   }
  
  if( !_taxonomy_node_term_node_access_node_privacy($tid) ) {
   return drupal_not_found();
   }
  
  
  
  // If feed type, hand it off the default taxonomy feed handling.
  if( 'feed' == arg(4) ){
   $depth = ( is_numeric(arg(3)) ) ? arg(3) : 0;
   return taxonomy_term_page($tid, $depth, 'feed');
   }
  
  // If Taxonomy Nodes are not enabled on this vocab, fall back on default taxonomy output...
  if( ! _taxonomy_node_is_vocab_enabled($term->vid) ){
   return taxonomy_term_page($tid, 0, 'page');
   }
   
   $current = $term;
   //$breadcrumbs = array( array('path' => $_GET['q'], 'title' =>$term->name) );
   $breadcrumbs = array( array('path' =>'taxonomy/term/'. $current->tid, 'title' =>$current->name) );
   while ($parents = taxonomy_get_parents($current->tid)) {
     $current = array_shift($parents);
     $breadcrumbs[] = array('path' => 'taxonomy/term/'. $current->tid, 'title' => $current->name);
     }
   $breadcrumbs = array_reverse($breadcrumbs);
   menu_set_location($breadcrumbs);
   
   $title = check_plain($term->name);
   drupal_set_title($title);
   
   $termNodes = _taxonomy_node_get_term_nodes($tid);
   
   $output = '';
   if( !empty($termNodes) ){
    foreach( $termNodes as $i => $node ){
      if( $node->status ){
       $output .= node_view($node, FALSE, TRUE, TRUE);
       }
      }
     }
   else{
    $nodeTypes = _taxonomy_node_get_content_types();
    $t = 0;  // TODO: How to offer a choice if multiple types are enabled??
    $links['taxonomy_node_edit'] = array(
     'title' => t('Edit').' '.check_plain($term->name),
     'href' => 'node/add/'.str_replace('_', '-', $nodeTypes[$t]).'/term/'.$term->tid,
     'attributes' => array('title' => t('Edit').' '.check_plain($term->name) ),
     );
    $output .= '<div class="links">'. theme('links', $links) .'</div>';
    }
   
   /* Include the normal taxonomy/term output */
   $depth = 0;
   $output .= taxonomy_render_nodes(taxonomy_select_nodes(array($tid), 'or', $depth, TRUE));
   drupal_add_feed( url('taxonomy/term/'. $tid .'/'. $depth .'/feed'), 'RSS - '. $title );
   
   return $output;
}







/**
 * Implementation of hook_db_rewrite_sql
 *
 * Returns An array of join statements, where statements, distinct decision.
 */
function taxonomy_node_db_rewrite_sql($query, $primary_table, $primary_field, $args) {
//   taxonomy_node_debug('fourDnet_db_rewrite_sql(); $primary_table: '.$primary_table.'; $primary_field: '.$primary_field.'; $args: '.print_r($args, true).'; $query: '.$query );
//   return;
  switch ($primary_field) {
    case 'tid':
      global $user;
      //if( $user->uid != 1 && module_exists('taxonomy_node') ){
      if( module_exists('taxonomy_node') ){
       $ret = array();
       $ret['join'] = "LEFT JOIN {taxonomy_node} txyn ON txyn.term_id = $primary_table.tid";
       $ret['where'] = "txyn.term_id IS NULL OR txyn.private = '0'";
       if( $user->uid ){
        $ret['where'] .= " OR txyn.uid = '".$user->uid."'";
        }
       return $ret;
       }
    break;
    }
}


