<?php


function _taxonomy_node_get_blocks($op = 'list', $delta = 0, $edit = array()) {
  return array(
          'taxonomy_node_node_tools' => 'Taxonomy Node Block',
          'taxonomy_node_term_tools' => 'Taxonomy Node Term Block',
          );
}


/**
 * Implementation of hook_block().
 */
function taxonomy_node_block($op = 'list', $delta = 0, $edit = array()) {
  switch ($op) {
    case 'list':
      $config = _taxonomy_node_get_blocks();
      $blocks = array();
      foreach ($config as $tid => $name) {
        $blocks[$tid]['info'] = t($name);
      }
     return $blocks;
    case 'view':
      $block = _taxonomy_node_blocks($op, $delta, $edit);
      return $block;
  }
}




function _taxonomy_node_blocks($op = 'view', $delta = 0, $edit = array()){
  if( $op == 'view' ){
   switch ($delta) {
    
    // Taxonomy Node Tools Block...
    case 'taxonomy_node_node_tools':
      return _taxonomy_node_node_tools_block($op, $delta, $edit);
    break;

    // Taxonomy Node Map (taxonomy_map.module) Tools Block...
    case 'taxonomy_node_term_tools':
      return _taxonomy_node_term_tools_block($op, $delta, $edit);
    break;
    }
   }
  
}


function _taxonomy_node_node_tools_block($op = 'view', $delta = 0, $edit = array()){

  $links = array();

   /* ------------------------------------------
    * Taxonomy / Node tools...
    */
  if( arg(0) == 'node' && is_numeric(arg(1)) ){
   $node = node_load(arg(1));
   // Only if it's a Taxonomy Node content type...
   if( _taxonomy_node_is_content_type($node->type) ) {
    $links[] = l('Return to '.check_plain($node->title), 'taxonomy/term/'.$node->taxonomy_node_term_id, array('title' => 'Return to '.check_plain($node->title) ));
    }
   }


   /* ------------------------------------------
    * Taxonomy / Term tools...
    */
  elseif( arg(0) == 'taxonomy' && arg(1) == 'term' && is_numeric(arg(2)) ){
   
   $term = taxonomy_get_term(arg(2));

   // Only if it's a Taxonomy Node vocabulary...
   if( $term && _taxonomy_node_is_vocab_enabled( $term->vid ) ){
    
    $nodeTypes = _taxonomy_node_get_content_types();
    $t = 0;  // TODO: How to offer a choice if multiple types are enabled??
    $nodeType = node_get_types('type', $nodeTypes[0]);
    
    $termNodes = _taxonomy_node_get_term_nodes( $term->tid );
    // If we already have a node for this term, offer a link to edit it (or them)...
    if( !empty($termNodes) ){
     $links[] = l(t('Edit').' '.check_plain($termNodes[0]->title), 'node/'.$termNodes[0]->nid.'/edit', array('title' => t('Edit').' '.check_plain($termNodes[0]->title)) );
     }
    
    // If no node exists for this term, offer a link to add one...
    else{
     $links[] = l(t('Edit').' '.check_plain($term->name), 'node/add/'.str_replace('_', '-', $nodeType->type).'/term/'.$term->tid, array('title' => t('Edit').' '.check_plain($term->name)) );
     }
    
    // Add a link to create a new Taxonomy Page
    // Is kind of confusing for users! Moved to separate block: _taxonomy_node_map_tools_block
    //$links[] = l('Add new '.drupal_ucfirst($nodeType->name), 'node/add/'.str_replace('_', '-', $nodeType->type).'/'.$term->vid, array('title' => 'Add new '.drupal_ucfirst($nodeType->name)) );
    }
   }
  
  if( !empty($links) ){
   // Assemble block parts...
   $blocks = _taxonomy_node_get_blocks();
   $block = array(
     'delta' => $delta,
     'subject' => $blocks[$delta],
     //'content' => theme('item_list', $links)
     'content' => theme('taxonomy_node_block_links', $links)
     );
   return $block;
   }
   
}


function _taxonomy_node_term_tools_block($op = 'view', $delta = 0, $edit = array()){

  $links = array();

   /* ------------------------------------------
    * Taxonomy / Term tools...
    */
  if( arg(0) == 'taxonomy' && arg(1) == 'term' && is_numeric(arg(2)) ){
   $term = taxonomy_get_term(arg(2));
   
   // Only if it's a Taxonomy Node vocabulary...
   if( $term && _taxonomy_node_is_vocab_enabled( $term->vid ) ){
    $nodeTypes = _taxonomy_node_get_content_types();
    $t = 0;  // TODO: How to offer a choice if multiple types are enabled??
    $nodeType = node_get_types('type', $nodeTypes[0]);
    // Add a link to create a new Taxonomy Node
    $links[] = l('Add new '.drupal_ucfirst($nodeType->name), 'node/add/'.str_replace('_', '-', $nodeType->type).'/'.$term->vid, array('title' => 'Add new '.drupal_ucfirst($nodeType->name)) );
    }
   }


  // Integrate with taxonomy_map module
  elseif( arg(0) == 'site-map' && module_exists('taxonomy_map') && is_numeric(arg(1)) ) {
    $nodeTypes = _taxonomy_node_get_content_types();
    $t = 0;  // TODO: How to offer a choice if multiple types are enabled??
    $nodeType = node_get_types('type', $nodeTypes[0]);
    $links[] = l('Add new '.drupal_ucfirst($nodeType->name), 'node/add/'.str_replace('_', '-', $nodeType->type).'/'.arg(1), array('title' => 'Add new '.drupal_ucfirst($nodeType->name)) );
   }


  if( !empty($links) ){
   // Assemble block parts...
   $blocks = _taxonomy_node_get_blocks();
   $block = array(
     'delta' => $delta,
     'subject' => $blocks[$delta],
     //'content' => theme('item_list', $links)
     'content' => theme('taxonomy_node_block_links', $links)
     );
   return $block;
   }
   
}




function theme_taxonomy_node_block_links($links){
  return theme('item_list', $links);
}







