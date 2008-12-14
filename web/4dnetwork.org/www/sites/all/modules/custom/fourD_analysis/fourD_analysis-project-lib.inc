<?php



/**
 * Add the Analysis Info to the node view
 */
function _fourD_analysis_project_nodeview(&$node){
  if( user_access('access analysis') ){
   /* Consolidated with table (form) below - 2008-07-10 */
   // $node->analysis = _fourD_analysis_project_analysis_summary( array($node->nid) );
//    }
//   if( user_access('submit project analysis') ){
   //$node->user_analysis = _fourD_analysis_add_user_analysis($node);]
   $node->user_analysis = drupal_get_form('fourD_analysis_project_analysis_inline_form', $node);
   }
}



 /**  Get the Prerequisite Projects for the given Project nid.  */
function _fourD_analysis_get_prerequisite_nodes($prereq_ids=array()) {
  
  $pre_goals = array();
  foreach( $prereq_ids as $i=>$nid ){
    $node = node_load($nid);
    if( isset($node->nid) ){
     $pre_goals[$node->nid] = $node;
     }
    }
  
  return $node_goals;
}




 // ***************************************************************** //
 // ****************           DB Functions           *************** //
 // ***************************************************************** //



/**
 * Get an analysis entry for an nid, and optionally a uid.
 * 
 * $alg - may be one of 'avg' or 'weighted'
 * 
 * avg: straight up average of all user inputs
 * weighted: weighted average; where ALL fields are weighted arrording to the individuals ability to assess
 * 
 */
function fourD_analysis_project_entry_get($nid, $uid=false, $alg='avg') {
  
  $anal = false;
  
  if($uid){
   $anal = db_fetch_array(db_query("SELECT aid, nid, uid, assess_ability, person_years, probability, communal_difference, communal_ability, communal_interest, personal_difference, personal_ability, personal_interest, personal_deadline FROM {node_4dnetwork_analysis} WHERE nid = '%d' AND uid = '%d'", $nid, $uid));
   
  if( $anal && isset($anal['probability']) ){
   $goals = db_query("SELECT r.goalid, r.type, r.rating, n.title FROM {node_4dnetwork_analysis_rating} r INNER JOIN {node} n ON n.nid = r.goalid WHERE r.nid = '%d' AND r.uid = '%d' AND (r.type = 'project_goal' OR r.type = 'project_prereq')", $nid, $uid);
   while( $r = db_fetch_object($goals) ) {
     $anal[$r->type][$r->goalid] = array();
     $anal[$r->type][$r->goalid]['title'] = $r->title;
     $anal[$r->type][$r->goalid]['rating'] = $r->rating;
     }
   
   // Populate any missing inputs for the user...
   $goals = db_query("SELECT DISTINCT(r.goalid), r.type, n.title FROM {node_4dnetwork_analysis_rating} r INNER JOIN {node} n ON n.nid = r.goalid WHERE r.nid = '%d' AND (r.type = 'project_goal' OR r.type = 'project_prereq')", $nid);
   while( $r = db_fetch_object($goals) ) {
     if( !isset($anal[$r->type][$r->goalid]) ){
      $anal[$r->type][$r->goalid] = array();
      $anal[$r->type][$r->goalid]['title'] = $r->title;
      $anal[$r->type][$r->goalid]['rating'] = 0.0;
      }
     }
    }
   }
  
  else{
   // $alg = 'weighted';
   
   /** Old style - Simple Average **/
   if( $alg == 'avg' ){
    $anal = db_fetch_array(db_query("SELECT COUNT(*) AS user_count, SUM(assess_ability) AS assess_ability, AVG(person_years) AS person_years, AVG(probability) AS probability, AVG(communal_difference) AS communal_difference, AVG(communal_ability) AS communal_ability, AVG(communal_interest) AS communal_interest, AVG(personal_difference) AS personal_difference, AVG(personal_ability) AS personal_ability, AVG(personal_interest) AS personal_interest, AVG(personal_deadline) AS personal_deadline FROM {node_4dnetwork_analysis} WHERE nid = %d", $nid));
    
    if( $anal && isset($anal['probability']) ){
     $anal['nid'] = $nid;
     $goals = db_query("SELECT r.goalid, r.type, AVG(r.rating) AS rating, n.title FROM {node_4dnetwork_analysis_rating} r INNER JOIN {node} n ON n.nid = r.goalid WHERE r.nid = '%d' AND (r.type = 'project_goal' OR r.type = 'project_prereq') GROUP BY r.type,r.goalid", $nid);
     while( $r = db_fetch_object($goals) ) {
       $anal[$r->type][$r->goalid] = array();
       $anal[$r->type][$r->goalid]['title'] = $r->title;
       $anal[$r->type][$r->goalid]['rating'] = $r->rating;
       }
     }
    }
   
   /** Weighted Average - Where ALL fields are * by the individuals ability to assess **/
   else if( $alg == 'weighted' ){
    $users = array();
    //$userWeight = array();
    $usersAbility = 0.0;
    $res = db_query("SELECT aid, nid, uid, assess_ability, person_years, probability, communal_difference, communal_ability, communal_interest, personal_difference, personal_ability, personal_interest, personal_deadline FROM {node_4dnetwork_analysis} WHERE nid = %d", $nid);
     while( $r = db_fetch_array($res) ) {
       $users[ $r['uid'] ] = $r;
       $usersAbility += $r['assess_ability'];
       //$userWeight[$r->uid] = $r->assess_ability;
       }
     
    if( count($users) ){
     /*  Load all the goals & prerequisites  */
     $goals = db_query("SELECT r.uid, r.goalid, r.type, r.rating, n.title FROM {node_4dnetwork_analysis_rating} r INNER JOIN {node} n ON n.nid = r.goalid WHERE r.nid = '%d' AND (r.type = 'project_goal' OR r.type = 'project_prereq')", $nid);
     while( $r = db_fetch_object($goals) ) {
       $users[$r->uid][$r->type][$r->goalid] = array();
       $users[$r->uid][$r->type][$r->goalid]['title'] = $r->title;
       $users[$r->uid][$r->type][$r->goalid]['rating'] = $r->rating;
       }
//      fourD_analysis_debug('fourD_analysis_project_entry_get;  $users: '.print_r($users, true) );
     
     
     /*  Cycle the users and apply the weighting  */
     $userCount = count($users);
     $avgUserAbility = ($usersAbility/$userCount);
//      fourD_analysis_debug('fourD_analysis_project_entry_get;  $userCount: '.$userCount.';  $usersAbility: '.$usersAbility.';  $avgUserAbility: '.$avgUserAbility );
     
     $anal = array();
     $anal['user_count'] = $userCount;
     foreach($users as $uid=>$inputs){
       if( $inputs['assess_ability'] ){
        //$uidAbility = ( $inputs['assess_ability']/$usersAbility );
        $uidAbility = ( ($avgUserAbility*$inputs['assess_ability'])*$usersAbility );
//         fourD_analysis_debug('fourD_analysis_project_entry_get;  $uid: '.$uid.';  $inputs[assess_ability]: '.$inputs['assess_ability'].';  $uidAbility: '.$uidAbility );
        foreach($inputs as $key=>$input){
          
          if( $key == 'project_goal' || $key == 'project_prereq' ) {
           foreach($input as $gpid=>$gpinput){
             if( !isset($anal[$key]) ) {
              $anal[$key] = array();
              }
             if( !isset($anal[$key][$gpid]) ) {
              $anal[$key][$gpid] = array();
              }
             $anal[$key][$gpid]['title'] = $gpinput['title'];
             $anal[$key][$gpid]['rating'] += ($gpinput['rating']*$uidAbility);
             }
           }
          
          else{
           if( !isset($anal[$key]) ) {
            $anal[$key] = 0.0;
            }
            $anal[$key] += ($inputs[$key]*$uidAbility);
           }
          
          }
        }
       }
     }
//     fourD_analysis_debug('fourD_analysis_project_entry_get;  $anal: '.print_r($anal, true) );
    }
   
   }
  
 // fourD_analysis_debug('_fourD_analysis_get_analysis; $nid: ' .$nid .', $uid: '.$uid. '; $anal: '. print_r($anal, TRUE) );
  return $anal;
}



/**
 * Implementation of hook_insert()
 */
function fourD_analysis_project_entry_insert($uid, $node) {
  // drupal_set_message('fourD_analysis_project_entry_insert(); $uid: '.$uid.'; $node: '.print_r($node, true) );
//   fourD_analysis_debug('fourD_analysis_project_entry_insert; $uid: ' .$uid.'; $node: '.print_r($node, true) );
//   return;
  
  if(!$uid)
   return false;
  
    // update project weighting
//     $node->weighting = calculate_project_weight($node);

   db_query("INSERT INTO {node_4dnetwork_analysis} 
     (aid, nid, uid, assess_ability, person_years, probability, communal_difference, communal_ability, communal_interest, personal_difference, personal_ability, personal_interest, personal_deadline)
     VALUES ('NULL', %d, %d, %f, %f, %f, %f, %f, %f, %f, %f, %f, %d)",
	 $node->nid,
	 $uid,
	 _fourD_analysis_validate_decimal( $node->analysis['project']['assess_ability'] ),
	 _fourD_analysis_round_decimal( $node->analysis['project']['person_years'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['probability'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['communal_difference'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['communal_ability'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['communal_interest'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['personal_difference'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['personal_ability'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['personal_interest'] ),
	 _fourD_analysis_validate_date( $node->analysis['project']['personal_deadline'] )
	 );
    
    // find new id...
    //$newid = db_result(db_query("SELECT aid FROM {node_4dnetwork_analysis} WHERE nid = %d AND uid = %d", $node->nid, $uid));
    
     
     /*  Add ratings for each applicable prerequisite  */
     if( isset($node->analysis['project_prereq']) && $node->analysis['project_prereq'] ){
      foreach( $node->analysis['project_prereq'] as $pid=>$rating ){
        if( $pid == 'add' ){
         $nids = explode(',', $rating);
         foreach($nids as $i=>$enid){
           if( is_numeric($enid) ){
            _fourD_analysis_project_rating_insert($uid, $node, $enid, 'project_prereq');
            }
           }
         }
        else{
         _fourD_analysis_project_rating_insert($uid, $node, $pid, 'project_prereq');
         }
        }
       }
    
    /*  Add ratings for each applicable goal  */
     $goals = _fourD_analysis_get_goal_nodes($node->nid);
     foreach( $goals as $gid=>$g ){
       // fourD_analysis_debug('fourD_analysis_project_entry_insert; Goal: ' .$key.';  $node->$key: '.$node->$key );
       if( isset( $node->analysis['project_goal'][$gid] ) ) {
        _fourD_analysis_project_rating_insert($uid, $node, $gid, 'project_goal');
        }
       }
}






/**
 * Implementation of hook_update()
 */
function fourD_analysis_project_entry_update($uid, $node) {
  // drupal_set_message('fourD_analysis_project_entry_update(); $uid: '.$uid.'; $node: '.print_r($node, true) );
  
  if(!$uid)
   return false;
  
    // update project weighting
//     $node->weighting = calculate_project_weight($node);

    // update database
    db_query("UPDATE {node_4dnetwork_analysis} SET
	assess_ability = %f,
	person_years = %f,
	probability = %f,
	communal_difference = %f,
	communal_ability = %f,
	communal_interest = %f,
	personal_difference = %f,
	personal_ability = %f,
	personal_interest = %f,
	personal_deadline = %d

	WHERE nid = %d AND uid = %d",
	
	 _fourD_analysis_validate_decimal( $node->analysis['project']['assess_ability'] ),
	 _fourD_analysis_round_decimal( $node->analysis['project']['person_years'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['probability'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['communal_difference'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['communal_ability'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['communal_interest'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['personal_difference'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['personal_ability'] ),
	 _fourD_analysis_validate_decimal( $node->analysis['project']['personal_interest'] ),
	 _fourD_analysis_validate_date( $node->analysis['project']['personal_deadline'] ),
	
	 $node->nid,
	 $uid
         );
    
     /*  Add ratings for each applicable prerequisite  */
     // db_query("DELETE FROM {node_4dnetwork_analysis_rating} WHERE nid = %d AND uid = %d AND type = 'project_prereq'", $node->nid, $uid);
     if( isset($node->analysis['project_prereq']) && is_array($node->analysis['project_prereq']) ){
      foreach( $node->analysis['project_prereq'] as $pid=>$rating ){
        if( $pid == 'add' ){
         $nids = explode(',', $rating);
         foreach($nids as $i=>$enid){
           if( is_numeric($enid) ){
            if( _fourD_analysis_project_rating_exists($uid, $node->nid, $enid, 'project_prereq') ){
             _fourD_analysis_project_rating_update($uid, $node, $enid, 'project_prereq');
             }
            else{
             _fourD_analysis_project_rating_insert($uid, $node, $enid, 'project_prereq');
             }
            }
           }
         }
        else{
         if( _fourD_analysis_project_rating_exists($uid, $node->nid, $pid, 'project_prereq') ){
          _fourD_analysis_project_rating_update($uid, $node, $pid, 'project_prereq');
          }
         else{
          _fourD_analysis_project_rating_insert($uid, $node, $pid, 'project_prereq');
          }
         }
        }
       }

     /*  Update ratings for each applicable goal  */
     // $goals = _fourD_analysis_get_goals();
     $goals = _fourD_analysis_get_goal_nodes($node->nid);
     foreach( $goals as $gid=>$g ){
       if( isset( $node->analysis['project_goal'][$gid] ) ){
        
        // fourD_analysis_debug('fourD_analysis_project_entry_update; Goal: ' .$key.';  $node->$key: '.$node->$key );
        if( _fourD_analysis_project_rating_exists($uid, $node->nid, $gid, 'project_goal') ){
          _fourD_analysis_project_rating_update($uid, $node, $gid, 'project_goal');
          }
        else{
          _fourD_analysis_project_rating_insert($uid, $node, $gid, 'project_goal');
          }
        
        }
       }
}

 /* Internal helper function */
function _fourD_analysis_project_rating_exists($uid, $nid, $goalid, $type) {
  $exists = db_fetch_array( db_query("SELECT rating FROM {node_4dnetwork_analysis_rating}
        WHERE nid = %d AND uid = %d AND goalid = %d AND type = '%s'",
        $nid,
        $uid,
        $goalid,
        $type
        )
       );
  return ( isset($exists['rating']) ) ? true : false;
}

 /* Internal helper function */
function _fourD_analysis_project_rating_insert($uid, $node, $goalid, $type) {
   db_query("INSERT INTO {node_4dnetwork_analysis_rating}
     (rid, nid, uid, goalid, type, rating) VALUES ('NULL', %d, %d, %d, '%s', %f)",
       $node->nid,
       $uid,
       $goalid,
       $type,
       _fourD_analysis_validate_decimal( $node->analysis[$type][$goalid] )
       );
}

 /* Internal helper function */
function _fourD_analysis_project_rating_update($uid, $node, $goalid, $type) {
   db_query("UPDATE {node_4dnetwork_analysis_rating} SET rating = %f
       WHERE nid = %d AND uid = %d AND goalid = %d AND type = '%s'",
       _fourD_analysis_validate_decimal( $node->analysis[$type][$goalid] ),
       $node->nid,
       $uid,
       $goalid,
       $type
       );
}


/**
 * Alternative to #fourD_analysis_project_entry_update
 * - Automatically inserts a new entry if one does not yet exist.
 */
function fourD_analysis_project_entry_update_insert($uid, $node) {
  if(!$uid)
   return false;
  $existing = fourD_analysis_project_entry_get($node->nid, $uid);
  if( $existing ){
   return fourD_analysis_project_entry_update( $uid, $node );
   }
  else{
   return fourD_analysis_project_entry_insert( $uid, $node );
   }
}


/**
 * Implementation of hook_delete()
 */
function fourD_analysis_project_entry_delete(&$node) {
  
  //delete DB information related to the node we are deleting
  db_query('DELETE FROM {node_4dnetwork_analysis} WHERE nid = %d', $node->nid);
  db_query('DELETE FROM {node_4dnetwork_analysis_rating} WHERE nid = %d', $node->nid);
}







 // **************************************************************** //
 // ****************              Forms              *************** //
 // **************************************************************** //

/**
 * Is the given Goal allowed to have user values for the Given Project.
 * - Allowed input are:
 *   a) all top-level Goals (currently 4 of them)
 *   b) the Projects direct "parent Goal"
 */
function _fourD_analysis_is_goal_applicable_to_project_input($project_nid, $goal){
   // fourD_analysis_debug('_fourD_analysis_is_goal_applicable_to_project_input; $project_nid: ' .$project_nid .'; $goal: '. print_r($goal, TRUE) );
  
  if( $project_nid && is_numeric($project_nid) ){
   if( in_array('0', $goal['parents'], true) ){ // Is top level - always visible
    return true;
    }
   
   else{ // The Goal (Term ID) exisits in the $project Tags (Terms)
    $terms = taxonomy_node_get_terms($project_nid);
    
    if( array_key_exists($goal['tid'], $terms) ){
     //fourD_analysis_debug('_fourD_analysis_is_goal_applicable_to_project_input; $project_nid: ' .$project_nid .'; $goal: '. print_r($goal, TRUE).'; $terms: '. print_r($terms, TRUE) );
     return true;
     }
    }
   }
  return false;
}


/**
 * Is the given applicable to the Given Project.
 * - Applicable Goals are:
 *   a) all top-level Goals (current 4 of them)
 *   b) ALL "parent Goals" for the Project (all levels up, in the Goal tree)
 */
function _fourD_analysis_is_goal_applicable_to_project($project_nid, $goal){
   // fourD_analysis_debug('_fourD_analysis_is_goal_applicable_to_project; $project_nid: ' .$project_nid .'; $goal: '. print_r($goal, TRUE) );
  
  if( $project_nid && is_numeric($project_nid) ){
   if( in_array('0', $goal['parents'], true) ){ // Is top level - always visible
    return true;
    }
  
   else{ // The Goal (Term ID) exists in the $project Tags (Terms)
    $terms = taxonomy_node_get_terms($project_nid);
    //fourD_analysis_debug('_fourD_analysis_is_goal_applicable_to_project; $project_nid: ' .$project_nid .'; $goal: '. print_r($goal, TRUE).'; $terms: '. print_r($terms, TRUE) );
    
    if( array_key_exists($goal['tid'], $terms) ){
     return true;
     }
    
    foreach($terms as $tid=>$term){
      $parents = taxonomy_get_parents_all($tid);
//       fourD_analysis_debug('_fourD_analysis_is_goal_applicable_to_project; $tid: ' .$tid .'; $parents: '. print_r($parents, TRUE) );
      foreach($parents as $pid=>$parent){
        if( $parent->tid == $goal['tid'] ){
         return true;
         }
        }
      }
    }
   }
  
  return false;
}

/**
 * Is the given Goal in the parents of the Given Project.
 */
function _fourD_analysis_is_goal_parent_to_project($project_nid, $goal, $includeDirectParent=false ){
//    fourD_analysis_debug('_fourD_analysis_is_goal_parent_to_project; $project_nid: ' .$project_nid .'; $goal: '. print_r($goal, TRUE) );
  
    $terms = taxonomy_node_get_terms($project_nid);
    //fourD_analysis_debug('_fourD_analysis_is_goal_parent_to_project; $project_nid: ' .$project_nid .'; $goal: '. print_r($goal, TRUE).'; $terms: '. print_r($terms, TRUE) );
    
    foreach($terms as $tid=>$term){
      $parents = taxonomy_get_parents_all($tid);
      //fourD_analysis_debug('_fourD_analysis_is_goal_parent_to_project; $tid: ' .$tid .'; $parents: '. print_r($parents, TRUE) );
      foreach($parents as $pid=>$parent){
        if( $parent->tid == $goal['tid'] && ($includeDirectParent || $tid != $parent->tid ) ){
         return true;
         }
        }
      }
  
  return false;
}


function _fourD_analysis_project_header_info( $column ) {
  
  $info_nid = _fourD_analysis_get_column_info_nid();
  switch($column){
    
    case 'user' :
      return array('data' => 'User', 'class'=>'project project-user', 'title' => t('User') );
    
    case 'users' :
      return array('data' => 'Users', 'class'=>'project project-user', 'title' => t('Number of Users who have entered Analysis Inputs') );
    
    case 'project' :
      return array('data' => 'Project', 'class'=>'project project-title', 'title' => t('Project Title') );
    
    case 'user_cnt' :
      return array('data' => 'Persons', 'class'=>'project project-user_cnt', 'title' => t('Detailed breakdown of Project analysis by Individual') );
    
    case 'analytics' :
      return array('data' => 'Analytics', 'class'=>'project project-analytics', 'title' => t('Detailed breakdown of Project analysis by Individual') );
    
//     case 'weight' :
//       return (!$info_nid) ?
//        array('data' => 'Weight', 'class'=>'project project-weight', 'title' => t('Weight') ) : 
//        array('data' => l('Weight', 'node/'.$info_nid, array('title'=>'Weight'), NULL, 'project-weight'), 'class'=>'project project-weight', 'title' => t('Weight') );
    
    case 'weight_personal' :
      return (!$info_nid) ?
       array('data' => 'Weight (personal)', 'class'=>'project project-weight project-weight-personal', 'title' => t('Weight') ) : 
       array('data' => 'Weight ('. l('personal', 'node/'.$info_nid, array('title'=>'Weight (personal)'), NULL, 'project-weight-personal') .')', 'class'=>'project project-weight project-weight-personal', 'title' => t('Weight (personal)') );
    
    case 'weight_subtotal' :
      return (!$info_nid) ?
       array('data' => 'Weight (subtotal)', 'class'=>'project project-weight project-weight-subtotal', 'title' => t('Weight') ) : 
       array('data' => 'Weight ('. l('subtotal', 'node/'.$info_nid, array('title'=>'Weight (subtotal)'), NULL, 'project-weight-subtotal') .')', 'class'=>'project project-weight project-weight-subtotal', 'title' => t('Weight (subtotal)') );
    
    case 'weight_communal' :
      return (!$info_nid) ?
       array('data' => 'Weight (communal)', 'class'=>'project project-weight project-weight-communal', 'title' => t('Weight') ) : 
       array('data' => 'Weight ('. l('communal', 'node/'.$info_nid, array('title'=>'Weight (communal)'), NULL, 'project-weight-communal') .')', 'class'=>'project project-weight project-weight-communal', 'title' => t('Weight (communal)') );
    
    case 'weight_subtotal_communal' :
      return (!$info_nid) ?
       array('data' => 'Weight (subtotal/communal)', 'class'=>'project project-weight project-weight-communal', 'title' => t('Weight') ) : 
       array('data' => 'Weight ('. l('subtotal', 'node/'.$info_nid, array('title'=>'Weight (subtotal)'), NULL, 'project-weight-subtotal') .' / '. l('communal', 'node/'.$info_nid, array('title'=>'Weight (communal)'), NULL, 'project-weight-communal') .')', 'class'=>'project project-weight project-weight-communal', 'title' => t('Weight (communal)') );
    
    
    case 'assess_ability' :
      return (!$info_nid) ?
       array('data' => 'Ability to Assess [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal ability to assess this project (ie: level of personal expertise in regards to this project)')) :
       array('data' => l('Ability to Assess', 'node/'.$info_nid, array('title'=>'Personal ability to assess this project (ie: level of personal expertise in regards to this project)'), NULL, 'assess-ability') . ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal ability to assess this project (ie: level of personal expertise in regards to this project)'));
    
    case 'person_years' :
      return (!$info_nid) ?
       array('data' => 'Person Years [0-&infin;]', 'class'=>'project project-'.$column, 'title' => t('Person Years') )  : 
       array('data' => l('Person Years', 'node/'.$info_nid, array('title'=>'Person Years'), NULL, 'person-years'). ' [0-&infin;]', 'class'=>'project project-'.$column, 'title' => t('Person Years') );
    
    case 'probability' :
      return (!$info_nid) ?
       array('data' => 'Probability of Success [0-1]', 'class'=>'project project-'.$column, 'title' => t('Probability of Success') ) : 
       array('data' => l('Probability of Success', 'node/'.$info_nid, array('title'=>'Probability of Success'), NULL, 'probability-of-success'). ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Probability of Success') );
    
    
    
    case 'communal_difference' :
      return (!$info_nid) ?
       array('data' => 'Communal Difference [0-1]', 'class'=>'project project-'.$column, 'title' => t('Communal Difference') ) : 
       array('data' => l('Communal Difference', 'node/'.$info_nid, array('title'=>'Communal Difference'), NULL, 'communal-difference'). ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Communal Difference') );
    
    case 'communal_ability' :
      return (!$info_nid) ?
       array('data' => 'Communal Ability [0-1]', 'class'=>'project project-'.$column, 'title' => t('Communal Ability') ) : 
       array('data' => l('Communal Ability', 'node/'.$info_nid, array('title'=>'Communal Ability'), NULL, 'communal-ability'). ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Communal Ability') );
    
    case 'communal_interest' :
      return (!$info_nid) ?
       array('data' => 'Communal Interest [0-1]', 'class'=>'project project-'.$column, 'title' => t('Communal Interest') ) : 
       array('data' => l('Communal Interest', 'node/'.$info_nid, array('title'=>'Communal Interest'), NULL, 'communal-interest'). ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Communal Interest') );
    
    
    
    case 'communal_personal_difference' :
      return (!$info_nid) ?
       array('data' => 'Difference (communal/personal) [0-1]', 'class'=>'project project-'.$column, 'title' => t('Difference (communal/personal)') ) : 
       array('data' => 'Difference ('. l('communal', 'node/'.$info_nid, array('title'=>'Communal Difference'), NULL, 'communal-difference') .'/'. l('personal', 'node/'.$info_nid, array('title'=>'Personal Difference'), NULL, 'personal-difference') .')'. ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Difference (communal/personal)') );
    
    case 'communal_personal_ability' :
      return (!$info_nid) ?
       array('data' => 'Ability (communal/personal) [0-1]', 'class'=>'project project-'.$column, 'title' => t('Ability (communal/personal)') ) : 
       array('data' => 'Ability ('. l('communal', 'node/'.$info_nid, array('title'=>'Communal Ability'), NULL, 'communal-ability') .'/'. l('personal', 'node/'.$info_nid, array('title'=>'Personal Ability'), NULL, 'personal-ability') .')'. ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Ability (communal/personal)') );
    
    case 'communal_personal_interest' :
      return (!$info_nid) ?
       array('data' => 'Interest (communal/personal) [0-1]', 'class'=>'project project-'.$column, 'title' => t('Interest (communal/personal)') ) : 
       array('data' => 'Interest ('. l('communal', 'node/'.$info_nid, array('title'=>'Communal Interest'), NULL, 'communal-interest') .'/'. l('personal', 'node/'.$info_nid, array('title'=>'Personal Interest'), NULL, 'personal-interest') .')'. ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Interest (communal/personal)') );
    
    
    
    case 'personal_difference' :
      return (!$info_nid) ?
       array('data' => 'Personal Difference [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal Difference') ) : 
       array('data' => l('Personal Difference', 'node/'.$info_nid, array('title'=>'Personal Difference'), NULL, 'personal-difference'). ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal Difference') );
    
    case 'personal_ability' :
      return (!$info_nid) ?
       array('data' => 'Personal Ability [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal Ability') ) : 
       array('data' => l('Personal Ability', 'node/'.$info_nid, array('title'=>'Personal Ability'), NULL, 'personal-ability'). ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal Ability') );
    
    case 'personal_interest' :
      return (!$info_nid) ?
       array('data' => 'Personal Interest [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal Interest') ) : 
       array('data' => l('Personal Interest', 'node/'.$info_nid, array('title'=>'Personal Interest'), NULL, 'personal-interest'). ' [0-1]', 'class'=>'project project-'.$column, 'title' => t('Personal Interest') );
    
    case 'personal_deadline' :
      return (!$info_nid) ?
       array('data' => 'Personal Deadline [yyyy-mm-dd]', 'class'=>'project project-'.$column, 'title' => t('Personal Deadline') ) : 
       array('data' => l('Personal Deadline', 'node/'.$info_nid, array('title'=>'Personal Deadline'), NULL, 'personal-deadline'). ' [yyyy-mm-dd]', 'class'=>'project project-'.$column, 'title' => t('Personal Deadline') );
    
    }
  return array('data' => 'Unknown Column');
}


/**
 * Get the Form Parts for Analysis input
 */
function _fourD_analysis_project_form_parts( $values=array(), $editable=true ) {
//    fourD_analysis_debug('_fourD_analysis_project_form_parts; $values: ' . print_r($values, TRUE) );
   
   $attrs = array();
   if( !$editable ){
    $attrs['disabled'] = 'disabled';
    }


     $form = array();
     
     // fieldset - analysis
     $form['analysis'] = array(
       '#title' => t('4D Network Analysis'),
       '#type' => 'fieldset',
       '#description' => t('Enter values for the various parameters. Explore the impact of ' .
                           'different values. Play around a get a feel for it.'),
       '#collapsible' => TRUE,
       '#collapsed' => FALSE,
       '#tree' => TRUE,
      );
     
     // fieldset - project_goal
     $weight = 1;
     $form['analysis']['project_goal'] = array(
       '#title' => t('Goal Weighting'),
       '#type' => 'fieldset',
       '#description' => t('Relative Weighting (arbitrary units) of this <em>Project</em> in relation to the repective <em>Goal</em>.'),
       '#collapsible' => FALSE,
       '#collapsed' => FALSE,
       '#weight' => 1
      );
     
     //$goals = _fourD_analysis_get_goals();
     $goals = _fourD_analysis_get_goal_nodes($values['nid']);
     // fourD_analysis_debug('_fourD_analysis_project_form_parts; $form_id: ' .$form_id .', #id: '.$form['#id']. '; #type: ' . $form['type']['#value'] .'; $goals: '. print_r($goals, TRUE) );
     
     foreach( $goals as $gid=>$g ){

       //if( _fourD_analysis_is_goal_applicable_to_project( $values['nid'], $g) ){
        $form['analysis']['project_goal'][$gid] = array(
          //'#title' => t('Goal '.$gid),
          '#title' => t($g['title']). ' [0-1]',
          '#type' => 'textfield',
          '#field_suffix' => t('[0.0]'),
          '#size' => 7,
          '#width' => 4,
          // '#default_value' => isset($values[$key]) ? number_format($values[$key], 10) : '0.0',
//           '#default_value' => _fourD_analysis_trim_decimal($values[$key]),  // Deprecated
          '#default_value' => _fourD_analysis_trim_decimal( $values['project_goal'][$gid]['rating'] ),
          '#attributes' => $attrs,
          '#weight' => $weight++
         );
        //}
       }



     // fieldset - project_prereq
     //$weight = 1;
     $form['analysis']['project_prereq'] = array(
       '#title' => t('Prerequisites'),
       '#type' => 'fieldset',
       '#description' => t('Relative Weighting (arbitrary units) of Project <em>Prerequisites</em>.'),
       '#collapsible' => FALSE,
       '#collapsed' => FALSE,
       '#weight' => 2
       );

     if( is_array($values['project_prereq']) ){
      foreach( $values['project_prereq'] as $pid=>$pre){
        $form['analysis']['project_prereq'][$pid] = array(
          '#title' => $pre['title']. ' [0-1]',
          '#type' => 'textfield',
          '#field_suffix' => t('[0.0]'),
          '#size' => 7,
          '#width' => 4,
          '#default_value' => _fourD_analysis_trim_decimal( $pre['rating'] ),
          '#attributes' => $attrs,
          '#weight' => $weight++
         );
        }
       }
     $form['analysis']['project_prereq']['add'] = array(
       '#title' => 'Add Prerequisite Project (optional; comma separated list of Project IDs)',
       '#type' => 'textfield',
       '#field_suffix' => t('Project IDs [1,2,3,4]...'),
       '#description' => t('Project IDs [1,2,3,4]...'),
       '#size' => 7,
       '#width' => 4,
       '#default_value' => '',
       '#attributes' => $attrs,
       '#weight' => $weight++
       );


     // fieldset - project
     //$weight = 1;
     $form['analysis']['project'] = array(
       '#title' => t('Project Analysis'),
       '#type' => 'fieldset',
       '#description' => t('Enter values for the various parameters. Explore the impact of ' .
                           'different values. Play around a get a feel for it.'),
       '#collapsible' => FALSE,
       '#collapsed' => FALSE,
       '#weight' => 2
      );
     
     $form['analysis']['project']['assess_ability'] = array(
       '#title' => t('Ability to Assess'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       '#description' => t('Enter your personal ability to assess this project (ie: your personal level of expertise in regards to this project)'),
       // '#default_value'=> isset($values['assess_ability']) ? number_format($values['assess_ability'], 10) : '0.0',
       '#default_value' => _fourD_analysis_trim_decimal( $values['assess_ability'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );

     $form['analysis']['project']['person_years'] = array(
       '#title' => t('Person Years'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0]'),
       '#size' => 4,
       '#width' => 4,
       // '#default_value' => isset($values['person_years']) ? number_format($values['person_years'], 10) : '0.0',
       '#default_value' => _fourD_analysis_trim_decimal( $values['person_years'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     $form['analysis']['project']['probability'] = array(
       '#title' => t('Probability of Success'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       // '#default_value'=> isset($values['probability']) ? number_format($values['probability'], 10) : '0.0',
       '#default_value' => _fourD_analysis_trim_decimal( $values['probability'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     
     
     
     
     $form['analysis']['project']['communal_difference'] = array(
       '#title' => t('Communal Difference'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       '#default_value' => _fourD_analysis_trim_decimal( $values['communal_difference'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     
     $form['analysis']['project']['communal_ability'] = array(
       '#title' => t('Personal Ability'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       '#default_value' => _fourD_analysis_trim_decimal( $values['communal_ability'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     
     $form['analysis']['project']['communal_interest'] = array(
       '#title' => t('Personal Interest'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       '#default_value' => _fourD_analysis_trim_decimal( $values['communal_interest'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     
     
     
     
     $form['analysis']['project']['personal_difference'] = array(
       '#title' => t('Personal Difference'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       // '#default_value'=> isset($values['personal_difference']) ? number_format($values['personal_difference'], 10) : '0.0',
       '#default_value' => _fourD_analysis_trim_decimal( $values['personal_difference'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     
     $form['analysis']['project']['personal_ability'] = array(
       '#title' => t('Personal Ability'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       // '#default_value'=> isset($values['personal_ability']) ? number_format($values['personal_ability'], 10) : '0.0',
       '#default_value' => _fourD_analysis_trim_decimal( $values['personal_ability'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     
     $form['analysis']['project']['personal_interest'] = array(
       '#title' => t('Personal Interest'),
       '#type' => 'textfield',
       '#field_suffix' => t('[0.0,1.0]'),
       '#size' => 4,
       // '#default_value'=> isset($values['personal_interest']) ? number_format($values['personal_interest'], 10) : '0.0',
       '#default_value' => _fourD_analysis_trim_decimal( $values['personal_interest'] ),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
     
     $deadline =  $values['personal_deadline'];
     $form['analysis']['project']['personal_deadline'] = array(
       '#title' => t('Personal Deadline'),
       '#type' => 'textfield',
       '#field_suffix' => t('YYYY-MM-DD'),
       '#size' => 4,
       '#default_value'=> isset($deadline) ? date('Y-m-d', $deadline) : date('Y-m-d'),
       '#attributes' => $attrs,
       '#weight' => $weight++
      );
  
  return $form;
}



/**
 * Implementation of hook_validate()
 */
function _fourD_analysis_project_form_parts_validate( $form=array(), $parts ) {
  // fourD_analysis_debug('_fourD_analysis_project_form_parts_validate; FORM: '. print_r($form, TRUE) );
  
//   $parts = _fourD_analysis_project_form_parts();
  
  /*  Validate the Project Goal Inputs  */
  foreach($parts['analysis']['project_goal'] as $key=>$vals){
    if( is_array($vals) && is_numeric($form['analysis']['project_goal'][$key]) ){
     $err = false;
      
      if( _fourD_analysis_validate_decimal( $form['analysis']['project_goal'][$key] ) === false ){
       $err = 'Please double check your input and try again.';
       }
     
     if( $err ){
      form_set_error($key, 'Invalid '.$vals['#title'].' input: '.$err);
      }
     }
    }
  
  
  /*  Validate the Project Prerequisite Inputs  */
  foreach($parts['analysis']['project_prereq'] as $key=>$vals){
    if( is_array($vals) ){
     $err = false;
      
     if( is_array($vals) ){

      /*  Check for new prerequisite node(s)  */
      if( $key == 'add' ){
       $nids = explode(',', $form['analysis']['project_prereq'][$key]);
       foreach($nids as $i=>$enid){
         if( is_numeric($i) && is_numeric($enid) ){
          $node = node_load( $enid );
          // Sanity check the prerequisite node(s).
          if( !$node ){
           $err = 'Invalid Prerequisite #'.(1+$i).';  ID: '. $enid.' does not exist';
           }
          else if( !_fourD_analysis_is_project_node_type($node->type) ){
           $err = 'Invalid Prerequisite #'.(1+$i).';  ID: '. $enid.' is not a valid Project';
           }
          }
         }
       }
      else{
       if( is_numeric($key) && is_numeric($form['analysis']['project_prereq'][$key]) ){
        $node = node_load( $key );
        // Sanity check the prerequisite node(s).
        if( !$node ){
         $err = 'Invalid Prerequisite #'.(1+$i).';  Project does not exist (ID: '.$key.').<br />  Please enter a valid Project ID or leave the input field blank to remove it from the equation.';
         }
        else if( !_fourD_analysis_is_project_node_type($node->type) ){
         $err = 'Invalid Prerequisite #'.(1+$i).';  ID: '. $key.' is not a valid Project';
         }
        if( _fourD_analysis_validate_decimal( $form['analysis']['project_prereq'][$key] ) === false ){
         $err = 'Please double check your input and try again.';
         }
        }
       }
      }
     
     if( $err ){
      form_set_error($key, 'Invalid '.$vals['#title'].' input: '.$err);
      }
     }
    }
  
  
  /*  Validate the rest of the Project Inputs  */
  foreach($parts['analysis']['project'] as $key=>$vals){
    if( is_array($vals) ){
     $err = false;
     
     /* Special case for Deadline */
     if( $key == 'personal_deadline' ){
      if( _fourD_analysis_validate_date( $form['analysis']['project'][$key] ) === false ){
       $err = 'Invalid Date.';
       }
      }
     
     /* Special case for Personal assess_ability (personal ability to assess must be > 0)  */
     else if( $key == 'assess_ability' ){
      $val = _fourD_analysis_validate_decimal( $form['analysis']['project'][$key] );
      $epsilon = 1e-11; // extra bit to satisfy validation check at a 1e-10 decimal precision
      if( $val === false || $epsilon > $val ){
       $err = 'Your personal ability to assess this Project must be greater than 0.0';
       }
      }
     
     /* Generic decimal fields */
     else{
      if( is_numeric($form['analysis']['project'][$key]) && _fourD_analysis_validate_decimal( $form['analysis']['project'][$key] ) === false ){
       $err = 'Please double check your input and try again.';
       }
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

$globalMultisortVar = array();
 function fourD_analysis_columnSort($recs, $cols=array('weight','desc') ) {
  //fourD_analysis_debug('fourD_analysis_columnSort; $cols: ' .$cols[0].','.$cols[1] );
     global $globalMultisortVar;
     $globalMultisortVar = $cols;
     usort($recs, 'fourD_analysis_multiStrnatcmp');
     return($recs);
 }
 
 function fourD_analysis_multiStrnatcmp($a, $b) {
     global $globalMultisortVar;
     $cols = $globalMultisortVar;
     $i = 0;
     $result = 0;
     while ($result == 0 && $i < count($cols)) {
         $result = ($cols[$i + 1] == 'desc' ? strnatcmp($b[$cols[$i]]['data'], $a[$cols[$i]]['data']) : $result = strnatcmp($a[$cols[$i]]['data'], $b[$cols[$i]]['data']));
         $i+=2;
     }
     return $result;
 }


/**
 * Add the Analysis Summary Info to the node view.
 * - Each user has their own analysis inputs.
 * - This table displays the averages from all user inputs.
 */
function _fourD_analysis_project_analysis_summary( $node_ids=array(), $uid=false, $title='Community Analysis', $analyticsLink=true, $nodeTitle=false){
  
//   $header = array();
//   if($nodeTitle){ // Add the Project Title on multiple project listing / summary.
//    $header[] = array('data' => 'Project', 'class'=>'project project-title');
//    }
//   else{
//    $header[] = array('data' => 'Users', 'class'=>'project project-user');
//    }
//   $header[] = array('data' => 'Assess Ability', 'class'=>'project');
//   $header[] = array('data' => 'Person Years', 'class'=>'project');
//   $header[] = array('data' => 'Probability', 'class'=>'project');
//   $header[] = array('data' => 'Difference', 'class'=>'project');
//   $header[] = array('data' => 'Ability', 'class'=>'project');
//   $header[] = array('data' => 'Interest', 'class'=>'project');
//   $header[] = array('data' => 'Deadline', 'class'=>'project');
//   $header[] = (count($node_ids) > 1) ?
//                  array('data' => 'Weight', 'sort'=>'desc', 'field'=>'weight', 'class'=>'project project-weight') :
//                  array('data' => 'Weight', 'class'=>'project project-weight');
//   
//   if($analyticsLink){ // Add the link to Detailed Project Analytics (user-by-user breakdown).
//    $header[] = array('data' => 'Analytics', 'class'=>'project project-analytics');
//    }
  
  
  global $user;
  
  $header = array();
  if($nodeTitle){ // Add the Project Title on multiple project listing / summary.
   $header[] = _fourD_analysis_project_header_info('project');
   }
  else{
   $header[] = _fourD_analysis_project_header_info('users');
   }
  $header[] = _fourD_analysis_project_header_info('assess_ability');
  $header[] = _fourD_analysis_project_header_info('person_years');
  $header[] = _fourD_analysis_project_header_info('probability');
//   $header[] = _fourD_analysis_project_header_info('personal_difference');
//   $header[] = _fourD_analysis_project_header_info('personal_ability');
//   $header[] = _fourD_analysis_project_header_info('personal_interest');
  if( false && $user->uid ){
   $header[] = _fourD_analysis_project_header_info('communal_personal_difference');
   $header[] = _fourD_analysis_project_header_info('communal_personal_ability');
   $header[] = _fourD_analysis_project_header_info('communal_personal_interest');
   }
  else{
   $header[] = _fourD_analysis_project_header_info('communal_difference');
   $header[] = _fourD_analysis_project_header_info('communal_ability');
   $header[] = _fourD_analysis_project_header_info('communal_interest');
   }
  $header[] = _fourD_analysis_project_header_info('personal_deadline');
  
  // $header[] = _fourD_analysis_project_header_info('pre_weight');
  $header[] = _fourD_analysis_project_header_info('weight_subtotal');
  $header[] = (count($node_ids) > 1) ?
                 array('data' => 'Weight', 'sort'=>'desc', 'field'=>'weight', 'class'=>'project project-weight project-weight-communal') :
                 //array('data' => 'Weight', 'class'=>'project project-weight');
                 _fourD_analysis_project_header_info('weight_communal');
  
  if( false && $user->uid ){
   $header[] = _fourD_analysis_project_header_info('personal_weight');
   }
  
  if($analyticsLink && user_access('access analytics') ){ // Add the link to Detailed Project Analytics (user-by-user breakdown).
   $header[] = _fourD_analysis_project_header_info('analytics');
   }
  
  // Build the list of Goal Headers...
  //$goals = array_reverse( _fourD_analysis_get_goals() );
  //$goals = array_reverse( _fourD_analysis_get_goal_nodes() );
  $headerGoals = array();
  $headerPrereqs = array();
  
  
  $analcache = array();
  foreach($node_ids as $i=>$nid){
    $goals = _fourD_analysis_get_goal_nodes($nid);
    foreach( $goals as $gid=>$g ){
      //fourD_analysis_debug('_fourD_analysis_project_analysis_summary; goal: '.$key.'; analysis['.$key.']: '.$analysis[$key] );
      if( !isset($headerGoals[$gid]) ){
       //$headerGoals[$gid] = array('data' => $g['title'], 'title' => 'Goal: '.$g['title'], 'depth'=>$g['depth'], 'class'=>'project-goal project-goal-l-' .$g['depth'] );
       $headerGoals[$gid] = array('data' =>l(check_plain($g['title']), 'taxonomy/term/'.$g['tid'], array('title'=>'Goal: '.check_plain($g['title'])) ). ' [0-1]' , 'title' => 'Goal: '.$g['title'], 'depth'=>$g['depth'], 'class'=>'project-goal project-goal-l-' .$g['depth'] );
       }
      }
  
    // Build the list of Prerequisite Headers...
    $analysis = fourD_analysis_project_entry_get($nid, $uid);
    if( $analysis && isset($analysis['probability']) ){
     $analcache[$nid] = $analysis;
     if( isset($analysis['project_prereq']) ){
      foreach( $analysis['project_prereq'] as $pid=>$p ){
        if( !isset($headerPrereqs[$pid]) ){
         //$headerPrereqs[$pid] = array('data' => $p['title'], 'title' => 'Prerequisite: '.$p['title'], 'class'=>'project-prereq' );
         $headerPrereqs[$pid] = array('data' => l(check_plain($p['title']), 'node/'.$pid, array('title'=>'Prerequisite: '.check_plain($p['title'])) ). ' [0-1]', 'title' => 'Prerequisite: '.$p['title'], 'class'=>'project-prereq' );
         }
        }
       }
     }
    }
   
  $rows = array();
  foreach($node_ids as $i=>$nid){
//     $analysis = fourD_analysis_project_entry_get($nid, $uid);
    $analysis = $analcache[$nid];
    //fourD_analysis_debug('_fourD_analysis_project_analysis_summary; $i: '.$i.'; $nid: '.$nid.'; $analysis: '.print_r($analysis, true) );
    if( $analysis && isset($analysis['probability']) ){
     
     $row = array();
     if($nodeTitle){
      $t = db_fetch_array(db_query('SELECT title FROM {node} WHERE nid = %d', $nid));
      $row[] = array(
                 // 'data' => check_plain($t['title']),
                 'data' => l(check_plain($t['title']), 'node/'.$nid, array('title'=>'Click to view '.check_plain($t['title'])) ),
                 'title'=>'View Project: '.check_plain($t['title']), 'class'=>'project project-title'
                 );
      }
     else{
      $row[] = array('data' => $analysis['user_count'], 'title'=>$analysis['user_count'].' Users have supplied Input', 'class'=>'project project-user' );
      }
     
     /*  Add the Users Ability to assess Column First  */
     $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['assess_ability'] ), 'class'=>'project project-assess_ability' );
     
     foreach( $headerGoals as $gid=>$g ){
       if( isset($analysis['project_goal']) ){
        if( isset($analysis['project_goal'][$gid]) ){
         $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['project_goal'][$gid]['rating'] ), 'class'=>'project-goal project-goal-l-' .$g['depth'] );
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project-goal project-goal-l-' .$g['depth'] );
         }
        }
       else{
        $row[] = array('data' => '-', 'class'=>'project-goal project-goal-l-' .$g['depth'] );
        }
       }
     
     foreach( $headerPrereqs as $pid=>$data ){
       if( isset($analysis['project_prereq']) ){
        if( isset($analysis['project_prereq'][$pid]) ){
         $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['project_prereq'][$pid]['rating'] ), 'class'=>'project-prereq' );
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project-prereq' );
         }
        }
       else{
        $row[] = array('data' => '-', 'class'=>'project-prereq' );
        }
       }
     
     //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['assess_ability'] ), 'class'=>'project' );
     $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['person_years'] ), 'class'=>'project' );
     $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['probability'] ), 'class'=>'project' );
     
     //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['personal_difference'] ), 'class'=>'project' );
     //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['personal_ability'] ), 'class'=>'project' );
     //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['personal_interest'] ), 'class'=>'project' );
     if( false && $user->uid ){
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_difference'] ) .'/'. _fourD_analysis_trim_decimal( $analysis['personal_difference'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_ability'] ) .'/'. _fourD_analysis_trim_decimal( $analysis['personal_ability'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_interest'] ) .'/'. _fourD_analysis_trim_decimal( $analysis['personal_interest'] ), 'class'=>'project' );
      }
     else{
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_difference'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_ability'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_interest'] ), 'class'=>'project' );
      }
     
     $row[] = array('data' => date('Y-m-d', $analysis['personal_deadline']), 'class'=>'project' );
     
     //$row[] = array('data' => _fourD_analysis_trim_decimal( _fourD_analysis_calculate_project_pre_weight($nid) ), 'class'=>'project project-weight' );
     
     $tmp = _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_pre_weight($nid));
     $row[] = array('data' => $tmp, 'class'=>'project project-weight project-weight-subtotal' );

     /*  Having the two weights in one columns messes up column sort :( */
     //$tmp = _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_pre_weight($nid));
     //$tmp .= ' / '. _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_weight($nid));
     $tmp =  _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_weight($nid));
     $row[] = array('data' => $tmp, 'class'=>'project project-weight project-weight-communal' );
     
     if( false && $user->uid ){
      $row[] = array('data' => _fourD_analysis_trim_decimal( _fourD_analysis_calculate_personal_project_weight($user->uid, $nid) ), 'class'=>'project project-weight' );
      }
     else{
      //$row[] = array('data' => _fourD_analysis_trim_decimal( _fourD_analysis_calculate_project_weight($nid) ), 'class'=>'project project-weight' );
      }
     
     
     if($analyticsLink && user_access('access analytics') ){
      $row[] = array('data' => l('View', 'project/'.$nid, array('title'=>'View '.check_plain($t['title']).' Analytics') ), 'class'=>'project project-analytics' );
      }
     
     $rows[] = $row;
     }
    }
  
  /*  Add the extra Prerequisite Headers  */
  $more = array_reverse($headerPrereqs, true);
  foreach( $more as $pid=>$data ){
    if($nodeTitle){
     array_splice($header, 2, 0, array($data));
     }
    else{
     //array_unshift($header, $data);
     array_splice($header, 2, 0, array($data));
     }
    }
  
  /*  Add the extra Goal Headers  */
  $more = array_reverse($headerGoals, true);
  foreach( $more as $gid=>$data ){
    if($nodeTitle){
     array_splice($header, 2, 0, array($data));
     }
    else{
     //array_unshift($header, $data);
     array_splice($header, 2, 0, array($data));
     }
    }

   
  // fourD_analysis_debug('_fourD_analysis_project_analysis_summary; sort: ' .$_GET['sort'] );
  $sort = ($_GET['sort'] == 'asc') ? 'asc' : 'desc';
  //$order_by = (isset($_GET['order']) && $_GET['order'] == 'Weight') ? $_GET['order'] : '6';

  $endOffset = ($analyticsLink && user_access('access analytics')) ? 2 : 1;
  $order_by = count($header)-$endOffset;
  $rows = fourD_analysis_columnSort($rows, array($order_by, $sort) );
  
  /*  Now tweak the weight output for scientific notation style
   *  - this must be done after the columnSort above.
   */
  $weightCol = count($header) - (($analyticsLink && user_access('access analytics')) ? 2 : 1);
  $weightSubCol = $weightCol-1;
  foreach($rows as $i=>$row){
    //fourD_analysis_debug('_fourD_analysis_project_analysis_summary; $weightCol: '.$weightCol.'; $rows['.$i.']['.$weightCol.'][data]: '.$rows[$i][$weightCol]['data'].'; '._fourD_analysis_scientific_notation($rows[$i][$weightCol]['data']) );
    $rows[$i][$weightSubCol]['data'] = _fourD_analysis_scientific_notation($rows[$i][$weightSubCol]['data']); // sub-total
    $rows[$i][$weightCol]['data'] = _fourD_analysis_scientific_notation($rows[$i][$weightCol]['data']);
    }
 
//   return theme_table($header, $rows, array(), $title);
  return theme('table', $header, $rows, array(), $title);
}









