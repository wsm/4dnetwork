<?php


function _node_section_get_blocks($op = 'list', $delta = 0, $edit = array()) {
  return array(
          'node_section_tools' => 'Node Sections',
          'node_section_contents' => 'Node Contents'
          );
}


/**
 * Implementation of hook_block().
 */
function node_section_block($op = 'list', $delta = 0, $edit = array()) {
  switch ($op) {
    case 'list':
      $config = _node_section_get_blocks();
      $blocks = array();
      foreach ($config as $tid => $name) {
        $blocks[$tid]['info'] = t($name);
      }
     return $blocks;
    case 'view':
      $block = _node_section_blocks($op, $delta, $edit);
      return $block;
  }
}




function _node_section_blocks($op = 'view', $delta = 0, $edit = array()){
  if( $op == 'view' ){
   switch ($delta) {
    
    // Node Sections Tools Block...
    case 'node_section_tools':
      return _node_section_tools_block($op, $delta, $edit);
    break;

    // Node Sections Contents Block...
    case 'node_section_contents':
      return _node_section_contents_block($op, $delta, $edit);
    break;
    }
   }
  
}



function _node_section_contents_block($op = 'view', $delta = 0, $edit = array()){

  $links = array();

   /* ------------------------------------------
    *  Node Section tools...
    */
  if( arg(0) == 'node' && is_numeric(arg(1)) ){
   $node = node_load(arg(1));
   
   // Only if it's a Node Sections enabled on this content type...
   if( _node_section_enabled($node) ) {
    $sections = _node_section_get_node_sections($node->nid, FALSE);
    if( !empty($sections) ){
//      foreach($sections as $anchor){
//        foreach($anchor as $sid=>$s){
//          $section = node_load($sid);
//          if( $section ){
//           $links[] = l(check_plain($section->title), 'node/'.$node->nid, array('title' => t('Goto ').' '.check_plain($section->title)), NULL, $section->title_anchor );
//           }
//          }
//         }
     foreach($sections as $anchor){
       foreach($anchor as $sid=>$s){
         $links[] = l(check_plain($s->title), 'node/'.$sid, array('title' => t('Goto ').' '.check_plain($s->title)), NULL, $s->anchor );
         }
        }
     }
    }
   
   }


  if( !empty($links) ){
   // Assemble block parts...
   $blocks = _node_section_get_blocks();
   $block = array(
     'delta' => $delta,
     'subject' => $blocks[$delta],
     //'content' => theme('item_list', $links)
     'content' => theme('node_section_contents_block_links', $links)
     );
   return $block;
   }
   
}



function _node_section_tools_block($op = 'view', $delta = 0, $edit = array()){

  $links = array();

   /* ------------------------------------------
    *  Node Section tools...
    */
  if( arg(0) == 'node' && is_numeric(arg(1)) ){
   $node = node_load(arg(1));
   
   // Only if it's a Node Sections enabled on this content type...
   if( _node_section_enabled($node) ) {
    $sectionTypes = _node_section_get_section_types();
    $type = node_get_types('type', $sectionTypes[0]);
    $links[] = l(t('Add ').' '.check_plain($type->name), 'node/add/'.str_replace('_', '-', $type->type).'/'.$node->nid, array('title' => t('Add ').' '.check_plain($type->name)) );
    }

   // Only if it's a Node Section content type...
   elseif( _node_section_is_section($node) && isset($node->section_node) ) {
    $parent = node_load( $node->section_node );
    $links[] = l(t('Return to ').' '.check_plain($parent->title), 'node/'.$parent->nid, array('title' => t('Return to ').' '.check_plain($parent->title)) );
    }
   
   }


  if( !empty($links) ){
   // Assemble block parts...
   $blocks = _node_section_get_blocks();
   $block = array(
     'delta' => $delta,
     'subject' => $blocks[$delta],
     //'content' => theme('item_list', $links)
     'content' => theme('node_section_tools_block_links', $links)
     );
   return $block;
   }
   
}




function theme_node_section_tools_block_links($links){
  return theme('item_list', $links);
}

function theme_node_section_contents_block_links($links){
  return theme('item_list', $links);
}







