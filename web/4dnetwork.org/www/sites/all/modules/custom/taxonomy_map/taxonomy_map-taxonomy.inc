<?php




/**
 * Helper function.
 */
function _taxonomy_map_add_css() {
  drupal_add_css( drupal_get_path('module', 'taxonomy_map') .'/styles.css' );
}

/**
 * Helper function for array_map purposes.
 */
function _taxonomy_map_get_vid_from_vocab($vocab) {
  return $vocab->vid;
}


// Build a taxonomy map...
function _taxonomy_map_get($op = 'view', $delta='rss', $edit = array()){
  if( $op == 'view' || $op == 'page' ){
   
   // Add the taxonomy_map css...
   _taxonomy_map_add_css();
   
   $crumb = false;
   $trees = array();

   // If no vacabs defined, we output all vocabs
   if ( empty($edit) || !$edit['vids'] ) {
    $vacabs = taxonomy_get_vocabularies();
    $edit['vids'] = implode(',', array_map('_taxonomy_map_get_vid_from_vocab', $vacabs));
    }
   
   //if( is_numeric($delta) ){
   if ( !empty($edit) && $edit['vids'] ) {
    $vids = explode(',', $edit['vids']);
    foreach($vids as $vid){
      if ( is_numeric($vid) ) {
       $vocab[$vid] = taxonomy_get_vocabulary($vid);
       if( $vocab[$vid] ) {
        if(!$crumb) $crumb = check_plain($vocab[$vid]->name);
        $trees[$vid] = taxonomy_get_tree($vocab[$vid]->vid);
        }
       }
      }
    }
   
   
   $breadcrumb = ($edit['breadcrumb']) ? $edit['breadcrumb'] : ((!$crumb)?'Site Map':$crumb);
   
   if( !empty($trees) ){
    
    if( $op == 'page' ){
     //drupal_set_title( check_plain($vocab->name) );
     drupal_set_breadcrumb( array( l(t('Home'), NULL), l(t($breadcrumb), $delta.'-map/'.$edit['vids'])) );
     $out = '';
     foreach($trees as $vid=>$tree){
       
       if( $edit['node-titles'] ){
        foreach ($tree as $term) {
         // taxonomy_map_debug('$term: '.print_r($term, true) );
          $term->nodes = theme('site_map_node_list', _taxonomy_map_select_nodes( array($term->tid), 'or', 0, TRUE) );
          }
        }
       
       $vars = array(
         'type'=>$delta,
         'terms'=>$tree,
         'title'=>$vocab[$vid]->name,
         'parent'=>'0',
         'collapsible'=>$edit['collapsible']
         );
       
       $out .= theme('site_map_page', $vars);
       }
     return $out;
     }
    else{ // if( $op == 'view' ){ // 'view' = block display
     $block['delta'] = $delta;
     $block['module'] = $delta.'-map';
     $block['content'] = '';
     foreach($trees as $vid=>$tree){
       
       $vars = array(
         'terms'=>$tree,
         'title'=>$vocab[$vid]->name,
         'parent'=>'0',
         'collapsible'=>$edit['collapsible']
         );
       
       $block['content'] .= theme('site_map_block', $vars);
       }
     return $block;
     }
    }
   else{
    if( $op == 'page' ){
     drupal_set_title( 'Invalid Category '.$edit['vids'] );
     drupal_set_breadcrumb( array( l(t('Home'), NULL), l(t($breadcrumb), $delta.'-map')) );
     return 'Invalid Category.';
     }
    }
   
   } // END if( $op == 'view' )
}

/**
 *
 * Borrowed from taxonomy.module#taxonomy_select_nodes - Hacked to include n_type in query
 *
 * Finds all nodes that match selected taxonomy conditions.
 *
 * @param $tids
 *   An array of term IDs to match.
 * @param $operator
 *   How to interpret multiple IDs in the array. Can be "or" or "and".
 * @param $depth
 *   How many levels deep to traverse the taxonomy tree. Can be a nonnegative
 *   integer or "all".
 * @param $pager
 *   Whether the nodes are to be used with a pager (the case on most Drupal
 *   pages) or not (in an XML feed, for example).
 * @param $order
 *   The order clause for the query that retrieve the nodes.
 * @return
 *   A resource identifier pointing to the query results.
 */
function _taxonomy_map_select_nodes($tids = array(), $operator = 'or', $depth = 0, $pager = TRUE, $order = 'n.sticky DESC, n.created DESC') {
  if (count($tids) > 0) {
    // For each term ID, generate an array of descendant term IDs to the right depth.
    $descendant_tids = array();
    if ($depth === 'all') {
      $depth = NULL;
    }
    foreach ($tids as $index => $tid) {
      $term = taxonomy_get_term($tid);
      $tree = taxonomy_get_tree($term->vid, $tid, -1, $depth);
      $descendant_tids[] = array_merge(array($tid), array_map('_taxonomy_get_tid_from_term', $tree));
    }

    if ($operator == 'or') {
      $args = call_user_func_array('array_merge', $descendant_tids);
      $placeholders = implode(',', array_fill(0, count($args), '%d'));
      $sql = 'SELECT DISTINCT(n.nid), n.type, n.sticky, n.title, n.created FROM {node} n INNER JOIN {term_node} tn ON n.nid = tn.nid WHERE tn.tid IN ('. $placeholders .') AND n.status = 1 ORDER BY '. $order;
      $sql_count = 'SELECT COUNT(DISTINCT(n.nid)) FROM {node} n INNER JOIN {term_node} tn ON n.nid = tn.nid WHERE tn.tid IN ('. $placeholders .') AND n.status = 1';
    }
    else {
      $joins = '';
      $wheres = '';
      $args = array();
      foreach ($descendant_tids as $index => $tids) {
        $joins .= ' INNER JOIN {term_node} tn'. $index .' ON n.nid = tn'. $index .'.nid';
        $placeholders = implode(',', array_fill(0, count($tids), '%d'));
        $wheres .= ' AND tn'. $index .'.tid IN ('. $placeholders .')';
        $args = array_merge($args, $tids);
      }
      $sql = 'SELECT DISTINCT(n.nid), n.type, n.sticky, n.title, n.created FROM {node} n '. $joins .' WHERE n.status = 1 '. $wheres .' ORDER BY '. $order;
      $sql_count = 'SELECT COUNT(DISTINCT(n.nid)) FROM {node} n '. $joins .' WHERE n.status = 1 '. $wheres;
    }
    $sql = db_rewrite_sql($sql);
    $sql_count = db_rewrite_sql($sql_count);
    if ($pager) {
      $result = pager_query($sql, variable_get('default_nodes_main', 10), 0, $sql_count, $args);
    }
    else {
      $result = db_query_range($sql, $args, 0, variable_get('feed_default_items', 10));
    }
  }

  return $result;
}







 ////////////////////////////////////////////////////////////////////////////////////////////
 /*  Theme Functions... */

/**
 * Theme the Taxonomy Term
 *
 * $vars['term']          - The Taxonomy Term Object
 * $vars['children']      - Optional Child elements
 * $vars['icon-feed']     - Optional Path to Feed Icon
 */
function theme_site_map_term($vars) {
  $out = '';
  if( $vars['term'] ) {
   $out .= '<li>';
   if( !is_null($vars['icon-collapse']) ){
    $alt = 'Expand / Collapse';
    $out .= '<a href="#" class="taxonomy-map-expander" title="'.$alt.'"> </a>';
    }
   if( !is_null($vars['icon-feed']) ){
    $alt = check_plain($vars['term']->name).' RSS Feed';
    $out .= l(' <img src="'.$vars['icon-feed'].'" alt="'.$alt.'" title="'.$alt.'"></img> ', 'taxonomy/term/'.$vars['term']->tid.'/0/feed', array(), NULL, NULL, TRUE, TRUE );
    }
   $out .= l(t($vars['term']->name), 'taxonomy/term/'.$vars['term']->tid, array('class'=>'map-category'));
   $out .= $vars['children'];

   // Add the node titles, if they exist...
   if( !is_null($vars['term']->nodes) && $vars['term']->nodes != '' ){
//     $out .= '<li>'.$vars['term']->nodes.'</li>';
    $out .= $vars['term']->nodes;
    }
   $out .= '</li>'."\n";
   }
  return $out;
}

/**
 * Theme the Taxonomy Map
 *
 * $vars - see params below.
 * $level - internal use only.
 *
 * $vars['terms']         - The Taxonomy Vocabulary array()
 * $vars['parent']        - Limit output to term under parent id
 * $vars['icon-feed']     - Optional Path to Feed Icon
 * $vars['icon-collapse'] - Optional Path to Collapse Icon
 *
 */
function theme_site_map_vocab($vars, $level=0) {
  $out = '';
  if (!empty($vars['terms'])) {
   
   foreach ($vars['terms'] as $term) {
     //$out .= '<li>' . $term . '</li>';
     //if( $term->parents[0] == $vars['parent'] ){
     if( in_array($vars['parent'], $term->parents, true) ){
      $items[] = theme('site_map_term', array('term'=>$term, 'icon-feed'=>$vars['icon-feed'], 'icon-collapse'=>$vars['icon-collapse'], 'children'=>theme('site_map_vocab', array('terms'=>$vars['terms'], 'parent'=>$term->tid, 'icon-feed'=>$vars['icon-feed'], 'icon-collapse'=>$vars['icon-collapse']), $level+1) ) );
      }
     }
   if( $items ){
    $hidden = ($level > 0) ? ' style="display:none;"' : '';
    $out = '<ul class="taxonomy-map"'.$hidden.' id="taxonomy-map-'.$vars['terms'][0]->vid.'-'.$vars['parent'].'">'."\n";
    $out .= implode("\n", $items);
    $out .= '</ul>'."\n"."\n";
    }
  }
  return $out;
}



/**
 * Theme the Taxonomy Map - in a Block: Title + RSS links
 *
 * $vars['title']       - Title of the Vocabulary Block
 * $vars['terms']       - The Taxonomy Vocabulary array()
 * $vars['parent']      - Limit output to term under parent id
 * $vars['collapsible'] - boolean - make the Vocabulary collapsible
 */
function theme_site_map_block($vars) {
  if( $vars['collapsible'] ){
   $js = '';
   }
  $ret = '<h2>'.check_plain($vars['title']).'</h2> ';
  $ret .= theme('site_map_vocab', array('terms'=>$vars['terms'], 'parent'=>$vars['parent'], 'collapsible'=>$vars['collapsible']) );
  return $ret;
}


/**
 * Theme the Taxonomy Map - in a Page: Title + RSS links
 *
 * $vars['type']        - Either 'site' or 'rss'
 * $vars['title']       - Title of the Vocabulary Block
 * $vars['terms']       - The Taxonomy Vocabulary array()
 * $vars['parent']      - Limit output to term under parent id
 * $vars['collapsible'] - boolean - make the Vocabulary collapsible
 */
function theme_site_map_page($vars) {
  // watchdog('theme_map_page', 'Tree: ' . print_r($vars['terms'], TRUE) );
  if( $vars['collapsible'] ){
   $elem = '#taxonomy-map-'.$vars['terms'][0]->vid.'-'.$vars['parent'];

   // ----------------------------------------------------
   $js = '$(document).ready(function() {
        // Collapse everything but the first term:
        $("'.$elem.' .taxonomy-map-expander").addClass("taxonomy-map-collapsed");
        $("'.$elem.' .taxonomy-map-expander").toggle(function() {
              $(this).removeClass("taxonomy-map-collapsed");
              $(this).addClass("taxonomy-map-expanded");
              return false;
            }, function() {
              $(this).removeClass("taxonomy-map-expanded");
              $(this).addClass("taxonomy-map-collapsed");
              return false;
            });
        // Expand or collapse:
        $("'.$elem.'").find(".taxonomy-map-expander").click(function() {
            $(this).find("~ ul").slideToggle("fast");
            return false;
        });
});';
    drupal_add_js($js, 'inline');
   }
  
  if($vars['type'] == 'rss' ){
   $vars['icon-feed'] = '/misc/xml.png';
   }
  
  if( $vars['collapsible'] ){
   $vars['icon-collapse'] = '/misc/xml.png';
   }
  
  drupal_set_title(''); // Clear the page title here, it's too much
  $ret = '<h2>'.check_plain($vars['title']).'</h2> ';
  $ret .= theme('site_map_vocab', array('terms'=>$vars['terms'], 'parent'=>$vars['parent'], 'icon-feed'=>$vars['icon-feed'], 'icon-collapse'=>$vars['icon-collapse']) );
  return $ret;
}




/**
 * Borrowed from theme.module#theme_item_list
 */
function theme_site_map_node_list($result) {
  $out = '';
  while ($node = db_fetch_object($result)) {
   $out .= '<li class="node-item">'. l($node->title, 'node/'. $node->nid, array('class'=>'map-node node-'.$node->type ) ) . '</li>'."\n";
   }
  if( $out != '' ){
   $out = '<ul class="node-list" style="display:none;">'.$out;
   $out .= '</ul>'."\n";
   }
  return $out;
}



























 ////////////////////////////////////////////////////////////////////////////////////////////
 /*  More Theme Functions... */

/**
 * Theme the Taxonomy Map
 */
function x_theme_rss_map_term_rss($term, $children) {
  $out = '';
  if($term) {
   $alt = check_plain($term->name).' RSS Feed';
   $out .= '<li>' .
            l(' <img src="/misc/xml.png" alt="'.$alt.'" title="'.$alt.'" width="36" height="14"> ', 'taxonomy/term/'.$term->tid.'/0/feed', array(), NULL, NULL, TRUE, TRUE ) .
            l(t($term->name), 'taxonomy/term/'.$term->tid) .
            $children .
            '</li>';
   }
  return $out;
}

/**
 * Theme the Taxonomy Map
 */
function x_theme_rss_map_tree_rss($tree, $parent=0) {
  $out = '';
  if (!empty($tree)) {
   
   foreach ($tree as $term) {
     if( $term->parents[0] == $parent ){
      if( !is_null($term->nodes) && $term->nodes != '<div class="item-list"></div>' ){
       $items[] = theme('rss_map_term_rss', $term, theme_rss_map_tree_rss($tree, $term->tid) ) . $term->nodes;
       }
      else{
       $items[] = theme('rss_map_term_rss', $term, theme_rss_map_tree_rss($tree, $term->tid) );
       }
      }
     }
   if( $items ){
    $out = '<ul class="taxonomy-map" id="taxonomy-map-'.$parent.'">'."\n";
    $out .= implode("\n", $items);
    $out .= '</ul>'."\n";
    }
  }
  return $out;
}



/**
 * Theme the Taxonomy Map - in a Block: Title + RSS links
 *
 * $precode - Offers an opportunity to prefix the output with some raw HTML/JS code
 */
function x_theme_map_block_rss($title, $tree, $parent=0, $precode='') {
  return $precode . '<h2>'.check_plain($title).'</h2> ' . theme_rss_map_tree_rss($tree, $parent);
}


/**
 * Theme the Taxonomy Map - in a Page: Title + RSS links
 *
 * $precode - Offers an opportunity to prefix the output with some raw HTML/JS code
 */
function x_theme_map_page_rss($title, $tree, $parent=0, $precode='') {
  return $precode . '<h2>'.check_plain($title).'</h2> ' . theme_rss_map_tree_rss($tree, $parent);
}






