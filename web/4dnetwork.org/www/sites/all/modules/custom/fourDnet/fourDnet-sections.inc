<?php
/* $Id: fourDnet-sections.module,v 1.2.2.4 2008/04/09 16:07:23 stickytouch.com Exp $ */
/**
 * @file
 * integrates with the node_sections.module (wiki style nodes).
 */
 



/**
 * Gets a list of page sections for Project Wikis.
 * Format:
 * #section: Unique section key (machine-readable: alphanumeric and - hyphens only)
 * #Title: The title of the Section
 * #Description: A description of the Section
 */
function _fourDnet_explode_raw_wiki_sections($template){
  //drupal_set_message('explode_raw_wiki_sections: $template: '.$template );
  $sections = array();
  
  if( isset($template) ){
   $rows = preg_split('/(\r\n|\n)/', $template );
   if( !empty($rows) ){
    $section = null;
    //drupal_set_message('explode_raw_wiki_sections: $rows: '. print_r($rows, true) );
    foreach($rows as $row){
      //drupal_set_message('explode_raw_wiki_sections: $row: '. $row );
      if( preg_match('/^\s*#section\s*:?/i', $row) ){
       $section = preg_replace('/^\s*#section\s*:?\s*/i', '', $row);
       //drupal_set_message('explode_raw_wiki_sections: $section: '.$section );
       $sections[$section] = array();
       }
      
      elseif( preg_match('/^\s*#title\s*:?/i', $row) ){
       $title = preg_replace('/^\s*#title\s*:?\s*/i', '', $row);
       //drupal_set_message('explode_raw_wiki_sections: $section: '.$section.'; $title: '.$title  );
       $sections[$section]['title'] = $title;
       }
      
      elseif( preg_match('/^\s*#description\s*:?/i', $row) ){
       $desc = preg_replace('/^\s*#description\s*:?\s*/i', '', $row);
       //drupal_set_message('explode_raw_wiki_sections: $section: '.$section.'; $desc: '.$desc  );
       $sections[$section]['description'] = $desc;
       }
      }
    }
   }
  return $sections;
}


/**
 * Gets a list of page sections for Project Wikis.
 */
function _fourDnet_get_project_wiki_sections_raw(){
  return variable_get('fourdnet_project_template', '');
}

/**
 * Gets a list of page sections for Goal Tree Wikis.
 */
function _fourDnet_get_goal_wiki_sections_raw(){
  return variable_get('fourdnet_goal_template', '');
}


/**
 * Gets a list of page sections for Project Wikis.
 */
function _fourDnet_get_project_wiki_sections_array(){
  static $sections = array();
  if( !count($sections) ){
   $sections = _fourDnet_explode_raw_wiki_sections( _fourDnet_get_project_wiki_sections_raw() );
   }
  return $sections;
}

/**
 * Gets a list of page sections for Goal Tree Wikis.
 */
function _fourDnet_get_goal_wiki_sections_array(){
  static $sections = array();
  if( !count($sections) ){
   $sections = _fourDnet_explode_raw_wiki_sections( _fourDnet_get_goal_wiki_sections_raw() );
   }
  return $sections;
}



/**
 * Gets a default template for a particular Project Wiki section.
 */
function _fourDnet_get_project_wiki_section($key){
  $sections = _fourDnet_get_project_wiki_sections_array();
  if( count($sections) && is_array($sections[$key]) ){
   return $sections[$key];
   }
  return false;
}


/**
 * Gets a default template for a particular Goal Wiki section.
 */
function _fourDnet_get_goal_wiki_section($key){
  $sections = _fourDnet_get_goal_wiki_sections_array();
  if( count($sections) && is_array($sections[$key]) ){
   return $sections[$key];
   }
  return false;
}







/** Output the Goal / Project sections, adding the default templates as configured */
function _fourDnet_output_sections(&$node, $configSections){
  //drupal_set_message('_fourDnet_output_goal_sections: $sections: '.print_r($node->sections, TRUE));
  
  $nodeSections = &$node->sections;
  
  $output = '';
  
  // Display all the configured sections first...
  $configSet = array();
//   $configSections = _fourDnet_get_goal_wiki_sections();
  foreach($configSections as $key=>$section){
    if( $nodeSections[$key] ){
     foreach($nodeSections[$key] as $sid=>$s){
       $configSet[$sid] = $sid;
       $output .= theme('node_section', $sid);
       }
     }
    else{
     // Setup a dummy node for viewing...
     $dummy = new stdClass();
     $dummy->section_node_empty = 1; // Flag for template to know this is a dummy $node.
     $dummy->type = 'node_section';
     $dummy->url_type = str_replace('_', '-', $dummy->type);
     $dummy->section_node = $node->nid;
     $dummy->section_anchor = $key;
     $dummy->title = $configSections[$key]['title'];
     $dummy->body = $configSections[$key]['description'];
     //$dummy->teaser = $configSections[$key]['description'];
     $dummy->title_anchor = node_section_get_title_anchor($configSections[$key]['title']);
     $dummy->format = 1;
     $dummy->status = 1;
     $output .= _fourDnet_node_section_view($dummy);
     //drupal_set_message('dummy node: '. print_r($dummy, true) );
     }
    }

  if( is_array($nodeSections) && !empty($nodeSections) ){
   foreach($nodeSections as $anchor){
     foreach($anchor as $sid=>$s){
       if( !isset($configSet[$sid]) ){
        $output .= theme('node_section', $sid);
        }
       }
      }
    }
  return $output;
}





/** Theme the output of Wiki Sections */
function phptemplate_node_sections(&$node){
  //drupal_set_message('4d_phptemplate_node_sections: $node: '.print_r($node, TRUE));

  if( _fourDnet_is_project_group_type($node->type) ){
   return _fourDnet_output_sections($node, _fourDnet_get_project_wiki_sections_array() );
   }

  elseif( module_exists('taxonomy_node') && _taxonomy_node_is_content_type($node->type) ){
   return _fourDnet_output_sections($node, _fourDnet_get_goal_wiki_sections_array() );
   }
  return '';
}

function _fourDnet_node_section_view(&$node){
  return node_view($node, FALSE, TRUE, FALSE);
}

function phptemplate_node_section($sid){
  //drupal_set_message('4d_phptemplate_node_section: node_load: '.print_r(node_load($sid), TRUE));
  return _fourDnet_node_section_view(node_load($sid));
}





