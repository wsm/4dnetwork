<?php




 /**  */
function _fourD_analysis_get_nid_for_term( $tid ) {
  $termNodes = _taxonomy_node_get_term_node_ids($tid);
  // fourD_analysis_debug('_fourD_analysis_get_nid_for_term; $termNodes: '. print_r($termNodes, TRUE) );
  $nid = false;
  if( !empty($termNodes) ){
    $nid = $termNodes[0];
   }
  return $nid;
}

function _fourD_analysis_get_goal_by_term( $tid ) {
  $goals = _fourD_analysis_get_goals_from_tree();
  //return ( isset($goals) ) ? $goals[$tid] : false;
  if( !empty($goals) ){
   foreach( $goals as $i=>$g ){
//      if( $tid == $g['tid'] ){
     if( $tid == $g->tid ){
      return $g;
      }
     }
   }
  return false;
}

function _fourD_analysis_get_goal_by_node( $nid ) {
  $goals = _fourD_analysis_get_goal_nodes();
  //return ( isset($goals) ) ? $goals[$nid] : false;
  if( !empty($goals) ){
   foreach( $goals as $i=>$g ){
     if( $nid == $g['nid'] ){
      return $g;
      }
     }
   }
  return false;
}


// function _fourD_analysis_get_goal_weight( $gid ) {
//    $termNodes = _taxonomy_node_get_term_nodes($gid);
//    $weight = 0.0;
//    if( !empty($termNodes) ){
//     $anal = fourD_analysis_goal_entry_get($termNodes[0]->nid);
//     if( $anal ){
//      $weight = $anal['goal_weight'];
//      }
//     }
//   return $weight;
// }




 /** Internal:  Get the Taxonomy Tree Terms (Goals) */
function _fourD_analysis_get_goals_from_tree() {
  static $tree_goals = array();
  
  if( !count($tree_goals) ){
   //fourD_analysis_debug('_fourD_analysis_get_goals_from_tree; $tree_goals: '. print_r($tree_goals, TRUE) );
   //$tree_goals = array();
   $fourDnetVocab = _fourDnet_get_project_tree_vocabs();
   //fourD_analysis_debug('_fourD_analysis_get_goals_from_tree; $fourDnetVocabs: '. print_r($fourDnetVocabs, TRUE) );
   
   $vocab = taxonomy_get_vocabulary($fourDnetVocab);
   if( $vocab ){
    $depth = _fourD_analysis_get_goal_depth();
    if($depth < 1){
     $depth = NULL;
     }
    $tree = taxonomy_get_tree($vocab->vid, 0, -1, $depth );
    // fourD_analysis_debug('_fourD_analysis_get_goals_from_tree; $tree: '. print_r($tree, TRUE) );
    
//     foreach( $tree as $i=>$term ){
//       // tree_goals[] = array( 'tid'=>$term->tid, 'name' => $term->name );
//       $tree_goals[] = array( 'tid'=>$term->tid, 'name' => $term->name );
//       }
    $tree_goals = $tree;
    }
   }
  
  return $tree_goals;
}

 /**  Get the available Goals; Limited to the configured max-depth  */
function _fourD_analysis_get_goal_nodes($project_nid=0, $filterType='input') {
  
  static $node_goals = array();
  
  if( !count($node_goals) ){
    $tree_goals = _fourD_analysis_get_goals_from_tree();
    // fourD_analysis_debug('_fourD_analysis_get_goal_nodes; $tree_goals: '. print_r($tree_goals, TRUE) );
    
    if( !empty($tree_goals) ){
     foreach( $tree_goals as $t=>$term ){
       // fourD_analysis_debug('_fourD_analysis_get_goal_nodes; term: '. print_r($term, TRUE) );
       $nid = _fourD_analysis_get_nid_for_term($term->tid);
       // fourD_analysis_debug('_fourD_analysis_get_goal_nodes; term: tid:'.$term->tid.';  $nid: '.$nid  );
       if($nid){
        $node_goals[$nid] = array( 'nid'=>$nid, 'tid'=>$term->tid, 'title' => $term->name, 'depth' => $term->depth, 'parents' => $term->parents );
        }
       }
     }
   }
  
  if( $project_nid && count($node_goals) ){
   $filtered = array();
   foreach($node_goals as $gid=>$g){
     if( $filterType == 'input' ){
      if( _fourD_analysis_is_goal_applicable_to_project_input($project_nid, $g) ){
       $filtered[$gid] = $g;
       }
      }
     elseif( $filterType == 'all' ){
      if( _fourD_analysis_is_goal_applicable_to_project($project_nid, $g) ){
       $filtered[$gid] = $g;
       }
      }
     }
   return $filtered;
   }
  
  return $node_goals;
}













 /**  */
function _fourD_analysis_get_goals_old() {
//   return false;
//  /*
//   * Details on what a goal is are sketchy.
//   *
//   * My inital understanding was that "Goals" are
//   * those from the Goal Tree.  Marshall made clear
//   * these are seperate so I really don't fully understand
//   * what the goal is now. ahem.  This funtion exists to
//   * make it easier down the road as more details become
//   * clear.
//   *
//   * +++ UPDATE 2008-05-30 [Marshall] - This IS based on Goal tree after all. huh
//   *
//   */
//   $goals = array(
//              array('id'=>'1', 'title' => 'Goal 1'),
//              array('id'=>'2', 'title' => 'Goal 2'),
//              array('id'=>'3', 'title' => 'Goal 3')
//              );

  static $all_goals = array();
  
  if( empty($all_goals) ){
   //fourD_analysis_debug('_fourD_analysis_get_goals; $all_goals: '. print_r($all_goals, TRUE) );
   //$all_goals = array();
   $fourDnetVocab = _fourDnet_get_project_tree_vocabs();
   //fourD_analysis_debug('_fourD_analysis_get_goals; $fourDnetVocabs: '. print_r($fourDnetVocabs, TRUE) );
   
   $vocab = taxonomy_get_vocabulary($fourDnetVocab);
   if( $vocab ){
    $tree = taxonomy_get_tree($vocab->vid, 0, -1, _fourD_analysis_get_goal_depth() );
    //fourD_analysis_debug('_fourD_analysis_get_goals; $tree: '. print_r($tree, TRUE) );
    
    foreach( $tree as $i=>$term ){
      $nid = _fourD_analysis_get_nid_for_term($term->tid);
      if($nid){
       $all_goals[] = array( 'id'=>$nid, 'title' => $term->name );
       }
      }
    }
   }
  
  return $all_goals;
}

function _fourD_analysis_get_goal_old( $gid ) {

  $goals = _fourD_analysis_get_goals();
  //return ( isset($goals) ) ? $goals[$gid] : false;
  
  if( !empty($goals) ){
   foreach( $goals as $i=>$g ){
     if( $gid == $g['id'] ){
      return $g;
      }
     }
   }
  return false;
}







/**
 * Add the Analysis Info to the node view
 */
function _fourD_analysis_goal_nodeview(&$node){
   $node->analysis = _fourD_analysis_goal_analysis_summary($node);
   //$node->user_analysis = _fourD_analysis_add_user_analysis($node);
   $node->user_analysis = drupal_get_form('fourD_analysis_goal_analysis_inline_form', $node);
}

























 // ***************************************************************** //
 // ****************           DB Functions           *************** //
 // ***************************************************************** //


function fourD_analysis_goal_entry_get($nid, $uid=null) {
  
  $anal = false;
  
  if($uid){
   $anal = db_fetch_array(db_query("SELECT rating AS goal_weight FROM {node_4dnetwork_analysis_rating} WHERE nid = '%d' AND uid = '%d'", $nid, $uid));
   }

  else{
   $anal = db_fetch_array(db_query("SELECT AVG(rating) AS goal_weight FROM {node_4dnetwork_analysis_rating} WHERE nid = %d", $nid));
   }
  
 // fourD_analysis_debug('_fourD_analysis_get_analysis; $nid: ' .$nid .', $uid: '.$uid. '; $anal: '. print_r($anal, TRUE) );
  return $anal;
}







/**
 * Implementation of hook_insert()
 */
function fourD_analysis_goal_entry_insert($uid, $node) {
   //fourD_analysis_debug('fourD_analysis_goal_entry_insert; $uid: ' .$uid.';  $node: '.print_r($node, true) );
//   return;
   
    // update goal weighting
//     $node->weighting = calculate_goal_weight($node);

    db_query("INSERT INTO {node_4dnetwork_analysis_rating}
     (rid, nid, uid, goalid, type, rating)
     VALUES ('NULL', %d, %d, %d, '%s', %f)",
         $node->nid,
         $uid,
         0,
        'goal',
         _fourD_analysis_validate_decimal( $node->goal_weight )
         );
}


/**
 * Implementation of hook_update()
 */
function fourD_analysis_goal_entry_update($uid, $node) {
   //fourD_analysis_debug('fourD_analysis_goal_entry_update; $uid: ' .$uid.';  $node: '.print_r($node, true) );
 
    // update goal weighting
//     $node->weighting = calculate_goal_weight($node);

    // update database
    db_query("UPDATE {node_4dnetwork_analysis_rating} SET
	rating = %f
	WHERE nid = %d AND uid = %d",
	 _fourD_analysis_validate_decimal( $node->goal_weight ),
	 $node->nid,
	 $uid
         );
}

/**
 * Alternative to #fourD_analysis_goal_entry_update
 * - Automatically inserts a new entry if one does not yet exist.
 */
function fourD_analysis_goal_entry_update_insert($uid, $node) {
  //fourD_analysis_debug('fourD_analysis_goal_entry_update_insert; $uid: ' .$uid.';  $node: '.print_r($node, true) );
  
  $existing = fourD_analysis_goal_entry_get($node->nid, $uid);
  if( $existing ){
   return fourD_analysis_goal_entry_update( $uid, $node );
   }
  else{
   return fourD_analysis_goal_entry_insert( $uid, $node );
   }
}


/**
 * Implementation of hook_delete()
 */
function fourD_analysis_goal_entry_delete(&$node) {
  // fourD_analysis_debug('fourD_analysis_goal_entry_delete; $node: '.print_r($node, true) );
  
  //delete DB information related to the node we are deleting
  db_query('DELETE FROM {node_4dnetwork_analysis_rating} WHERE nid = %d', $node->nid);
}




 // **************************************************************** //
 // ****************              Forms              *************** //
 // **************************************************************** //


/**
 * Get the Form Parts for Analysis input
 */
function _fourD_analysis_goal_form_parts( $values=array() ) {
  // fourD_analysis_debug('_fourD_analysis_goal_form_parts; $form_id: ' .$form_id .', #id: '.$form['#id']. '; #type: ' . $form['type']['#value'] .'; FORM: '. print_r($form, TRUE) );
     
     $form = array();
     
     // fieldset
     $form['analysis'] = array(
       '#title' => t('Goal Analysis'),
       '#type' => 'fieldset',
       '#description' => t('Enter values for the various parameters. Explore the impact of ' .
                           'different values. Play around a get a feel for it.'),
       '#collapsible' => TRUE,
       '#collapsed' => FALSE,
      );
     
     $form['analysis']['goal_weight'] = array(  // piggy back off the goal_weight from Ratings schema
       '#title' => t('Weight'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       // '#default_value'=> isset($values['goal_weight']) ? number_format($values['goal_weight'], 10) : '0.0',
       '#default_value' => _fourD_analysis_trim_decimal( $values['goal_weight'] ),
       '#weight' => 1,
      );
  
  return $form;
}




/**
 * Implementation of hook_validate()
 */
function _fourD_analysis_goal_form_parts_validate( $form=array() ) {
  // fourD_analysis_debug('_fourD_analysis_goal_form_parts_validate; FORM: '. print_r($form, TRUE) );
  
  $parts = _fourD_analysis_goal_form_parts();
  
  foreach($parts['analysis'] as $key=>$vals){
    if( is_array($vals) ){
     $err = false;

//       if( _fourD_analysis_validate_decimal( $form['#post'][$key] ) === false ){
      if( _fourD_analysis_validate_decimal( $form[$key] ) === false ){
       $err = 'Please double check your input and try again.';
       }
     
     if( $err ){
      form_set_error($key, 'Invalid '.$vals['#title'].' input: '.$err);
      }
     }
    }
  
  //form_set_error('del_goals', 'Testing - STOP');
}







 // ****************************************************************** //
 // ************          node_view Summary & Forms        *********** //
 // ****************************************************************** //


/**
 * Add the Analysis Summary Info to the node view.
 * - Each user has their own analysis inputs.
 * - This table displays the averages from all user inputs.
 */
function _fourD_analysis_goal_analysis_summary(&$node){
  
  $header = array(
              array('data' => 'Weight')
              );
  
  $analysis = fourD_analysis_goal_entry_get($node->nid);
  if( $analysis ){
   $rows[] = array(
              array('data' => _fourD_analysis_trim_decimal( $analysis['goal_weight'] ) )
              );
   }
  
//   return theme_table($header, $rows, array(), 'Overall Community Weighting');
  return theme('table', $header, $rows, array(), 'Overall Community Weighting');
  //fourD_analysis_debug('_fourD_analysis_add_user_analysis; $node->analysis: ' .$node->analysis );
}



 // ****************************************************************** //
 // ************            node_view Inline Form          *********** //
 // ****************************************************************** //



/**
 * Add the User Analysis Info to the node view.
 * - Each user has their own analysis inputs.
 * - This form allows users to add/edit their personal inputs.
 */
function fourD_analysis_goal_analysis_inline_form($node){
  // fourD_analysis_debug('add_inline_form(); $node: '.$node->nid  );
  
  global $user;
  if( $user->uid ){
   
   $values = fourD_analysis_goal_entry_get($node->nid, $user->uid);
   $form = _fourD_analysis_goal_form_parts( $values );

//      $form['analysis'] = array(
//        '#title' => t('Goal Analysis'),
//        '#type' => 'fieldset',
//        '#description' => t('Enter values for the various parameters. Explore the impact of ' .
//                            'different values. Play around a get a feel for it.'),
//        '#collapsible' => TRUE,
//        '#collapsed' => FALSE,
//       );
   
   $form['analysis']['#title'] = 'My Personal Weighting';
   $form['analysis']['#description'] = null;
   
   $form['analysis']['#theme'] = 'fourD_analysis_goal_analysis_inline_form_table';
   $form['analysis']['submit'] = array('#type' => 'submit', '#value' => t('Save') );
   
   //$node->user_analysis = theme_table($header, $rows, array(), 'My Personal Analysis');
   //fourD_analysis_debug('_fourD_analysis_add_user_analysis; $node->user_analysis: ' .$node->user_analysis );
   
   return $form;
   }
}

function fourD_analysis_goal_analysis_inline_form_validate( $formid, $form ){
  // fourD_analysis_debug('add_inline_form_validate; $form: '.print_r($form, true) );

  if( arg(0) == 'node' && is_numeric(arg(1)) ){
   $node = node_load( arg(1) );
   }

  elseif( arg(0) == 'taxonomy' && is_numeric(arg(2)) ){
   $nid = _fourD_analysis_get_nid_for_term(arg(2));
   if( $nid ){
    $node = node_load($nid);
    }
   }

  if( !$node || !isset($node->nid) ){
   form_set_error('nid', 'Invalid node link');
   return false;
   }

   _fourD_analysis_goal_form_parts_validate( $form );
}

function fourD_analysis_goal_analysis_inline_form_submit( $formid, $form){
  // fourD_analysis_debug('add_inline_form_submit nid: '.arg(1).'; $form: '.print_r($form, true) );
  
  if( arg(0) == 'node' && is_numeric(arg(1)) ){
   $node = node_load( arg(1) );
   }

  elseif( arg(0) == 'taxonomy' && is_numeric(arg(2)) ){
   $nid = _fourD_analysis_get_nid_for_term(arg(2));
   if( $nid ){
    $node = node_load($nid);
    }
   }

  if( !$node || !isset($node->nid) ){
   drupal_set_error('Invalid node link');
   return false;
   }

  
  // Add the form parts to the node, for saving below...
  $parts = _fourD_analysis_goal_form_parts();
  foreach( $parts['analysis'] as $key => $value ) {
     if( is_array($value) ){
     $node->$key = $form[$key];
     // fourD_analysis_debug('fourD_analysis_goal_analysis_inline_form_submit;  $key: '.$key.';  $value: '.$node->$key );
     }
    }
  
  global $user;
  /* Auto Update/Insert DB function */
  fourD_analysis_goal_entry_update_insert($user->uid, $node);
  
}

/**
 * Some hackery to coerce the form into a tidy table.
 *
 * Used at the bottom of Analysis items for adding/updating
 * personal analysis inputs.
 */
function theme_fourD_analysis_goal_analysis_inline_form_table(&$form){
  // fourD_analysis_debug('theme_fourD_analysis_goal_analysis_inline_form_table');

   $header = array(
              array('data' => 'Weight'),
              array('data' => '')
              );
  
  $rows = array(array());
  $submit = '';
  $other = '';
  $parts = _fourD_analysis_goal_form_parts();
  foreach($form as $key => $value) {
    if( is_array($value) ){

     if( isset($parts['analysis'][$key]) || $key == 'submit'){
      //fourD_analysis_debug('theme_fourD_analysis_goal_analysis_inline_form_table;  $key: '.$key.';  $value: '.print_r($value, true) );
      $tmp = drupal_render($form[$key]);
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      if( $key == 'submit'){
       $submit = $tmp;
       }
       else{
       $rows[0][] = array('data' => $tmp. ' [0-1]' );
       }
      }
     else{
      $other .= drupal_render($form[$key]) . "\n";
      }
     }
    }
  $rows[0][] = array('data' => $submit );

//   return theme_table($header, $rows) . $other;
  return theme('table', $header, $rows) . $other;
}











