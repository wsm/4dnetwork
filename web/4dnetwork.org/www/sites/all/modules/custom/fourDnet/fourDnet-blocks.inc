<?php



// ----------------------------------------
// Theme a block array...
function fourDnet_render_block($block = array()){
  return (!$block) ? "" : _phptemplate_callback('block', array('block' => array2object($block) ) );
}


/**
 * Implementation of hook_block().
 */
function fourDnet_block($op = 'list', $delta = 0, $edit = array()) {
  $gblocks = array(
      // 4D Blocks...
     'project_tools' => 'Project/Goal Tools',
     );
  switch ($op) {
    case 'list':
      $blocks = array();
      foreach ($gblocks as $tid => $name) {
        $blocks[$tid]['info'] = t($name);
      }
     return $blocks;
    case 'view':
      $block = _fourDnet_blocks($op, $delta, $edit);
      return $block;
  }
}








function _fourDnet_blocks($op = 'view', $delta = 0, $edit = array()){
  if( $op == 'view' ){
   switch ($delta) {
    
    // ------------------------------------
    // Menu Blocks...
    case 'project_tools':
      // include_once(drupal_get_path('module', 'fourDnet') .'/blocks/menu.inc');
      return _fourDnet_project_tools_block($op, $delta, $edit);
    break;
    }
   }
  
}



function _fourDnet_project_tools_block($op = 'view', $delta = 0, $edit = array()){
  // fourDnet_debug('$op: '.$op );
  
  $links = array();
  
  $baseLink = '';
  if( arg(0) == 'node' && is_numeric(arg(1)) ){
   $node = node_load(arg(1));
   }
  
  if( arg(0) == 'taxonomy' && arg(1) == 'term' && is_numeric(arg(2)) ){
   $term_nodes = _taxonomy_node_get_term_nodes(arg(2), false);
   if( count($term_nodes) ){
    $node = node_load( $term_nodes[0]->sid );
    }
   }
  
  
  
  if( !$node->nid ){
   return;
   }
  else{
   $baseLink = (arg(0) == 'node') ? 'node/'.$node->nid : 'taxonomy/term/'.arg(2);

  if( _node_section_enabled($node) ) {
   

//    $nodeSections = _node_section_get_node_sections($node->nid, FALSE);
   $nodeSections = _node_section_get_node_sections($node->nid, false);
   
   $title = '';
   $configSections = array();
   
   // Project Group type...
  if( _fourDnet_is_project_group_type($node->type) ){
   //$title = check_plain($node->title);
   $title = 'Project Wiki';
   $configSections = _fourDnet_get_project_wiki_sections_array();
   }
  
  // Goal type...
  elseif( module_exists('taxonomy_node') && _taxonomy_node_is_content_type($node->type) ){
   // $title = check_plain($node->title);
   $title = 'Goal Wiki';
   $configSections = _fourDnet_get_goal_wiki_sections_array();
   }
  
  
  
  $sections = $configSections;
  if( isset($nodeSections) && count($nodeSections) ){
   $sections = array_merge($configSections, $nodeSections);
   }
  
  if( count($sections) ){
   foreach($sections as $key=>$section){
     // fourDnet_debug('_fourDnet_project_tools_block('.$op.'); $nid: '.$node->nid.'; $sections: '.print_r($sections, true) );
     if( isset($nodeSections[$key]) ){
      foreach($nodeSections[$key] as $sid=>$s){
        $links[] = l(check_plain($s->title), $baseLink, array('title' => t('Goto ').' '.check_plain($s->title)), NULL, $s->anchor );
        }
      }
     else{
      $links[] = l(check_plain($section['title']), $baseLink, array('title' => t('Goto ').' '.check_plain($section['title'])), NULL, $key );
      }
     }
   }
  }
  
  
  // Add a link to add new Node Section...
  // Only if it's a Node Sections enabled on this content type...
  if( _node_section_enabled($node) ) {
   $sectionTypes = _node_section_get_section_types();
   $type = node_get_types('type', $sectionTypes[0]);
   $links[] = l(t('New').' '.check_plain($type->name), 'node/add/'.str_replace('_', '-', $type->type).'/'.$node->nid, array('title' => t('New').' '.check_plain($type->name)) );
   }

  // Node Section content type...
  elseif( _node_section_is_section($node) && isset($node->section_node) ) {
   $parent = node_load( $node->section_node );
   $links[] = l(t('Return to ').' '.check_plain($parent->title), 'node/'.$parent->nid, array('title' => t('Return to ').' '.check_plain($parent->title)) );
   }
   
  if( !empty($links) ){
   // Assemble block parts...
   $block = array(
     'delta' => $delta,
     'subject' => $title,
     'content' => theme('fourDnet_block_links', $links)
     );
   return $block;
   }







//    // ------------------------------------------
//    // Project Group type...
//    if( _fourDnet_is_project_group_type($node->type) ){
//      
//     $projectInfoTypes     = _fourDnet_get_project_info_node_types();
//     $projectTopicTypes    = _fourDnet_get_project_topic_node_types();
//     $projectTemplateTypes = _fourDnet_get_project_template_types();
//     $projectTemplates     = _fourDnet_get_project_nodes( $node->nid );
//     
//     foreach( $projectTemplateTypes as $type => $title ){
//       if( $projectTemplates[$type] ){
//        $links[] = l(t('View Project '.$title), 'node/'.$projectTemplates[$type][0]->nid, array('title'=>t('View the '.$title.' page for '.$node->title) ));
//        }
//       else{
//       if( user_access('create project info pages') ){
//         $n = 0;  // TODO: Are we allowing multiple page page types for pages??
//         $links[] = l(t('Add Project '.$title), 'node/add/'.$projectInfoTypes[$n].'/'.$node->nid.'/'.$type, array('title'=>t('Add a '.$title.' page for '.$node->title) ));
//         }
//        }
//       }
//    
//     $n = 0;  // TODO: Are we allowing multiple page page types for pages??
//     $links[] = l(t('Add New Topic'), 'node/add/'.$projectTopicTypes[$n].'/'.$node->nid, array('title'=>t('Add a Topic to '.$node->title) ));
//     $links[] = l(t('View Topics'), 'project-tracker/'.$node->nid, array('title'=>t('View Topics in '.$node->title) ));
// 
//     // Assemble block parts...
//     $block = array(
//       'delta' => $delta,
//       'subject' => "Project Tools",
//       'content' => theme('item_list', $links)
//       );
//      return $block;
//      }

//    // ------------------------------------------
//    // Project Page Node type...
//    elseif( _fourDnet_is_project_info_node_type($node->type) || _fourDnet_is_project_topic_node_type($node->type) ) {
// 
//     // Load the Parent Project...
//     _fourDnet_get_node_project($node);
//     
// //     $links[] = l(t('Return to @name', array('@name'=>$node->project->title) ), 'node/'.$node->project->nid, array('title' => t('Return to Project.')));
//     $links[] = l('Return to '.$node->project->title, 'node/'.$node->project->nid, array('title' => t('Return to Project.')));
//     
//     // Assemble block parts...
//     $block = array(
//       'delta' => $delta,
//       'subject' => "Project Tools",
//       'content' => theme('item_list', $links)
//       );
//     return $block;
//     }

//    // ------------------------------------------
//    // Project Tree Page Node type...
//    elseif( _fourDnet_is_project_tree_node_type($node->type) ) {
// 
//     // Load the Parent Project Term...
//     _fourDnet_get_term_for_term_node($node);
//     
//     $links[] = l(t('Return to @name', array('@name'=>$node->project_tree_term->name) ), 'taxonomy/term/'.$node->project_tree_term->tid, array('title' => t('Return to Goal Tree.')));
//     
//     // Assemble block parts...
//     $block = array(
//       'delta' => $delta,
//       'subject' => "Goal Tools",
//       'content' => theme('item_list', $links)
//       );
//     return $block;
//     }

   }


//   // ---- Project Tree / Term tools...
//   if( arg(0) == 'taxonomy' && arg(1) == 'term' && is_numeric(arg(2)) ){
//    
//    $term = taxonomy_get_term(arg(2));
//    if( !$term || !_fourDnet_is_project_tree_vocab( $term->vid ) ){
//     return;
//     }
//    
//    $links[] = array();
//    
//    $termNodes = _fourDnet_get_project_tree_nodes( $term->tid );
//    // If we already have a node for this term, offer a link to edit it (or them)...
//    if( isset( $termNodes['summary'] ) ){
//     $links[] = l(t('Edit Goal Information'), 'node/'.$termNodes['summary'][0]->nid.'/edit', array('title' => t('Edit the current Goal Information.')));
//     }
//    
//    // If no node exists for this term, offer a link to add one...
//    else{
//     $treeTypes = _fourDnet_get_project_tree_node_types();
//     $n = 0;  // TODO: Are we implementing multiple pages for terms??
//     $links[] = l(t('Add Goal Information'), 'node/add/'.$treeTypes[$n].'/'.$term->tid, array('title' => t('Add information for current Goal.')) );
//     }
//    
//    // Assemble block parts...
//    $block = array(
//      'delta' => $delta,
//      'subject' => "Goal Tools",
//      'content' => theme('item_list', $links)
//      );
//    return $block;
//    }

}

function theme_fourDnet_block_links($links){
  return theme('item_list', $links);
}




