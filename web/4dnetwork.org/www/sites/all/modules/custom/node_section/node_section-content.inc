<?php

/**
 * Implementation of hook_perm().
 * @return array
 */
function node_section_perm() {
  return array(
          'access node sections',
          'create node sections',
          'edit node sections',
          );
}


/**
 * Implementation of hook_node_info().
 */
function node_section_node_info() {
  return array(
    'node_section' => array(
      'name'        => t('Node Section'),
      'module'      => 'node_section_content',
      'description' => t('Create a new node section.'),
      'help'        => t('Create a new node section.'),
      'body_label'  => t('Description'),
    ),
  );
}


/**
 *  Implementation of hook_access().
 */
function node_section_content_access($op, $node) {
  switch($op) {
  	case 'view':
  	  return user_access('access node sections');
  	  break;
  	case 'create':
  	case 'delete':
  	  return user_access('create node sections');
  	  break;
  	case 'update':
  	  return user_access('edit node sections');
  	  break;
  }
}




/**
 * Implementation of hook_form().
 */
function node_section_content_form(&$node, &$param) {

  $node_types = _node_section_get_enabled_types();
  
  if( !$node_types ){
   drupal_set_message('Warning: Node Sections are not configured.', 'error');
   return;
   }
  
  $title = '';
  $excludes = array();
  
  // Special case for CCK / manage fields
  if( !isset($node->cck_dummy_node_form) ){
  /* --- Add new node --- */
  if( !$node->nid ){
   if( is_numeric(arg(3)) ){
    $parent = node_load(arg(3));
    if( !_node_section_enabled($parent) ){
     drupal_set_message('This type of content does not support Sections.', 'error');
     return;
     }
    $anchor = (arg(4)) ? preg_replace('/[^a-zA-Z0-9\-]/is', '', arg(4)) : 'general';
    $form['node_section']['node_section_anchor'] = array(
      '#type'  => 'hidden',
      '#id'    => 'node-section-anchor',
      '#name'  => 'node_section_anchor',
      '#value' => $anchor
       );
    $form['node_section']['node_section_node'] = array(
      '#type'  => 'hidden',
      '#id'    => 'node-section-node',
      '#name'  => 'node_section_node',
      '#value' => $parent->nid
       );
    $form['node_section']['node_section_parent_type'] = array(
      '#type'  => 'hidden',
      '#id'    => 'node-section-parent-type',
      '#name'  => 'node_section_parent_type',
      '#value' => $parent->type
       );
    $form['#redirect'] = 'node/'.$parent->nid;
    /*  Integrate with taxonomy_node module */
    if( module_exists('taxonomy_node') ){
     $toxo_nid = _taxonomy_node_get_tid_for_nid($parent->nid);
     if( $toxo_nid ){
      $form['#redirect'] = 'taxonomy/term/'.$toxo_nid;
      }
     }
    }
   else{
    drupal_set_message('No parent for section.', 'error');
    return;
    }
   }
  else{
   $form['#redirect'] = 'node/'.$node->section_node;
   
   /*  Integrate with taxonomy_node module */
   if( module_exists('taxonomy_node') ){
    $toxo_nid = _taxonomy_node_get_tid_for_nid($node->section_node);
    if( $toxo_nid ){
     $form['#redirect'] = 'taxonomy/term/'.$toxo_nid;
     }
    }
   }
   }
  
  
  // TODO: How to allow templates to show a default title & description?
  $title = $node->title;
  $body = $node->body;
  
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
    '#default_value' => $body,
    '#required' => FALSE,
    '#rows' => 20
  );
  $form['body_filter']['filter'] = filter_form($node->format);
  
  return $form;
}



/**
 * Implementation of hook_delete().
 */
function node_section_content_delete($node) {
  db_query("DELETE FROM {node_sections} WHERE sid = %d", $node->nid);
}

/**
 * Implementation of hook_load().
 */
function node_section_content_load($node) {
  if( _node_section_is_section($node->type) ){
   $add = db_fetch_object(db_query("SELECT nid AS section_node, anchor AS title_anchor FROM {node_sections} WHERE sid = %d", $node->nid));
   //$add->title_anchor = node_section_get_title_anchor($node->title);
   //$add->title_anchor = $node->anchor;
   return $add;
   }
}


/**
 * Implementation of hook_validate().
 */
function node_section_content_validate($node, &$form) {
  if( !$node->nid && (!$node->node_section_node || !is_numeric($node->node_section_node)) ){
   form_set_error('title', 'Invalid Parent');
   }
}

/**
 * Implementation of hook_submit().
 */
function node_section_content_submit(&$node) {
  //node_section_debug('node_section_content_submit();  $node: '.print_r($node, true) );
  //_node_section_content_save('submit', $node);
}

/**
 * Implementation of hook_insert().
 */
function node_section_content_insert($node) {
  _node_section_content_save('insert', $node);
}


/**
 * Save the Term Node.
 */
function _node_section_content_save($hook, &$node) {
  if($hook == 'update'){
//    if( !$node->node_section_anchor || $node->node_section_anchor == 'general' ){
//     $node->node_section_anchor = node_section_get_title_anchor($node->title);
//     }
//    $sql = "UPDATE {node_sections} SET weight='%d', anchor='%s' WHERE sid = '%d'";
//    db_query($sql, 0, $node->node_section_anchor, $node->nid );
   }
  elseif($hook == 'insert'){
   if( !$node->node_section_anchor || $node->node_section_anchor == 'general' ){
    $node->node_section_anchor = node_section_get_title_anchor($node->title);
    }
   $sql = "INSERT INTO {node_sections} (sid,nid,weight,anchor) VALUES ('%d', '%d', '%d', '%s')";
   db_query($sql, $node->nid, $node->node_section_node, 0, $node->node_section_anchor );
   }
}


/**
 * Implementation of hook_link().
 */
function node_section_link($type, $node = NULL, $teaser = FALSE) {
  
  if( $type == 'node' && _node_section_is_section($node->type) ) {
   $links['node_section_edit'] = array(
    'title' => t('Edit').' '.check_plain($node->title),
    'href' => 'node/'.$node->nid.'/edit',
    'attributes' => array('title' => t('Edit').' '.check_plain($node->title) ),
    );
   }
  
  return $links;
}








