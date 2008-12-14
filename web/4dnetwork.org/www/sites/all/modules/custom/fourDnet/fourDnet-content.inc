<?php



/**
 * Implementation of hook_perm().
 *
 * @return array
 */
function fourDnet_perm() {
  return array(
          'access projects',
          'create projects', 'create personal projects', 'create private projects', 'edit projects', 'edit own projects',
          'create project topic pages', 'edit project topic pages', 'edit own project topic pages',
          );
}



/**
 * Implementation of hook_node_info().
 */
function fourDnet_node_info() {
  return array(
    'project_group' => array(
      'name'        => t('Project'),
      'module'      => 'fourDnet_project_group',
      'description' => t('Create a new Project.'),
      'help'        => t('Create a new Project.'),
      'body_label'  => t('Description'),
    ),
    'project_topic' => array(
      'name'        => t('Project Topic Page'),
      'module'      => 'fourDnet_project_topic',
      'description' => t('Create a new Topic/Discussion within a Project.'),
      'help'        => t('Create a new Topic/Discussion within a Project.'),
      'body_label'  => t('Description'),
    ),
  );
}




/**
 *  Implementation of hook_access().
 */
function fourDnet_project_group_access($op, $node) {
  //drupal_set_message('fourDnet_project_group_access(); $op: '.$op.'; $node: '.print_r($node, true) );
  switch($op) {
  	case 'view':
  	  // return user_access('access projects') && _fourDnet_project_group_access_node_privacy($node);
  	  return user_access('access projects'); // Privacy check moved to nodeapi so we can return a 404 instead of 403
  	  break;
  	case 'create':
  	  return user_access('create projects');
  	  break;
  	case 'update':
  	case 'delete':
  	  global $user;
  	  return (
                  ( user_access('edit projects') && _fourDnet_project_group_access_node_privacy($node) ) ||
                  ( user_access('edit own projects') && $node->uid == $user->uid )
                 );
  	  break;
  }
}


/**
 *  Implementation of hook_access() - Extra Privacy control (beyond that of what Groups module does)
 */
function _fourDnet_project_group_access_node_privacy($node) {
  //drupal_set_message('_fourDnet_project_group_access_node_privacy(); $node: '.print_r($node, true) );
  
  global $user;
  return ( !$node->og_private || $node->uid == $user->uid );
}




/**
 *  Implementation of hook_access().
 */
function fourDnet_project_topic_access($op, $node) {
  switch($op) {
  	case 'view':
  	  return user_access('access projects');
  	  break;
  	case 'create':
  	case 'delete':
  	  return user_access('create project topic pages');
  	  break;
  	case 'update':
  	  return user_access('edit project topic pages');
  	  break;
  }
}


function fourDnet_project_group_form(&$node) {
  $type = node_get_types('type', $node);

  // We need to define form elements for the node's title and body.
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => TRUE,
    '#default_value' => $node->title,
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
  
  if( user_access('create personal projects') ){
   $form['is_personal'] = array(
     '#type' => 'checkbox',
     '#title' => 'Personal',
     '#required' => FALSE,
     '#default_value' => $node->taxonomy_node_is_personal,
     '#description' => 'If set this Project will be considered "Personal" rather than "Community".',
     //'#weight' => -5
     );

   /* This is handled by OG module (for now at least) */
   if( user_access('create private projects') ){
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
  

     // Pre-select the taxonomy term if provided...
     if( is_numeric(arg(3)) ){
      $vid = _fourDnet_get_project_tree_vocabs();
      $term = taxonomy_get_term(arg(3));
         if( $term->vid == $vid ) {
          //fourDnet_debug('form_alter; $possibility: '. print_r($term, TRUE) );
          $taxo = new stdClass();
          $taxo->tid = $term->tid;
          $taxo->vid = $term->vid;
          $taxo->name = $term->name;
          $taxo->description = $term->description;
          $node->taxonomy[$term->tid] = $taxo;
         }
      //fourDnet_debug('form_alter; $node: ' . print_r($node, TRUE) );
      }

  return $form;
}


function fourDnet_project_topic_form(&$node) {
  $type = node_get_types('type', $node);

  // We need to define form elements for the node's title and body.
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => TRUE,
    '#default_value' => $node->title,
    '#weight' => -5
  );
  // We want the body and filter elements to be adjacent; stuff them in a sub-array.
  $form['body_filter']['body'] = array(
    '#type' => 'textarea',
    '#title' => check_plain($type->body_label),
    '#default_value' => $node->body,
    '#required' => FALSE
  );
  $form['body_filter']['filter'] = filter_form($node->format);

  return $form;
}









/**
 * Implementation of hook_db_rewrite_sql
 *
 * Returns An array of join statements, where statements, distinct decision.
 */
function fourDnet_db_rewrite_sql($query, $primary_table, $primary_field, $args) {
//   fourDnet_debug('fourDnet_db_rewrite_sql(); $primary_table: '.$primary_table.'; $primary_field: '.$primary_field.'; $args: '.print_r($args, true).'; $query: '.$query );
//   return;
  switch ($primary_field) {
    
    case 'nid':
      global $user;
      //if( $user->uid != 1 && module_exists('taxonomy_node') ){
      if( module_exists('og') ){
       $ret = array();
       // $ret['join'] = "LEFT JOIN {og} npx ON $primary_table.nid = npx.nid";
       //$ret['where'] = "npx.nid IS NULL OR npx.private = '0'";
       $ret['join'] = "LEFT JOIN {node_4dnetwork} npx ON $primary_table.nid = npx.oid";
       $ret['where'] = "npx.oid IS NULL OR npx.private = '0'";
       if( $user->uid ){
        $ret['where'] .= " OR $primary_table.uid = '".$user->uid."'";
        }
       return $ret;
       }
      break;
  }
}














