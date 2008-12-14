<?php



 /* Round a decimal value to a 1e-10 decimal precision */
function _fourD_analysis_round_decimal( $d ) {
   $epsilon = 1e-11;
   return round( $d + $epsilon, 10 );
}

 /* Remove trailing 0 (zero) from a float */
function _fourD_analysis_trim_decimal( $d=null ) {
  $str = '0.0';
  if( $d != null && trim($d) != '' ){
   $str = number_format($d, 10);
   $str = preg_replace('/0+$/is', '', $str); // rip trailing 0's
   
//    fourD_analysis_debug('_fourD_analysis_trim_decimal('.$d.');  $str: '.$str.';  strripos($str, "."): ' .strripos($str, '.') .', strlen($str): '.strlen($str) );

   if( strripos($str, '.') == (strlen($str)-1) ){
    $str = $str.'0';
//    fourD_analysis_debug('_fourD_analysis_trim_decimal('.$d.');  $str: '.$str.'; ' .strripos($str, '.') .' == '.(strlen($str)-1) );
    }
//    fourD_analysis_debug('_fourD_analysis_trim_decimal('.$d.');  ret $str: '.$str.';  strripos($str, "."): ' .strripos($str, '.') .', strlen($str): '.strlen($str) );

   }
  return $str;
}


 /* Return a float Scientific Notation
  * - This may change, need to double check if output/format is acceptable.
  */
function _fourD_analysis_scientific_notation($d){
   // return _fourD_analysis_round_decimal($d);
   return sprintf("%.3e", $d); // scientific notation - this may change, confirm with Will
}



 /* Check that a decimal value is within range for analysis vars; 0.0 through 1.0 */
function _fourD_analysis_validate_decimal_old( $d ) {
  
  if( is_numeric($d) ){
   
   $min = 0.0;
   $max = 1.0;
   $epsilon = 1e-11; // extra bit to satisfy equality check of a 1e-10 decimal precision
   
   if( $d > $min + $epsilon && $d < $max - $epsilon ){
    return true;
    }
   }
  return false;
}

 /* Check that a decimal value is within range for analysis vars; 0.0 through 1.0 */
function _fourD_analysis_validate_decimal( $d ) {
  
  if( is_numeric($d) ){
   
   $rounded = _fourD_analysis_round_decimal($d);

   $min = 0.0;
   $max = 1.0;
   $epsilon = 1e-11; // extra bit to satisfy validation check at a 1e-10 decimal precision
   
   // fourD_analysis_debug('_fourD_analysis_validate_decimal('.$d.'); $rounded: '.$rounded.'; $min: '.$min.'; $max: '.$max.'; $epsilon: '.$epsilon );

   if( $rounded <= $min + $epsilon ){
    return $min;
    }
   elseif( $rounded >= $max - $epsilon ){
    return $max;
    }
   
   if( $rounded > $min + $epsilon && $rounded < $max - $epsilon ){
    return $rounded;
    }
   }
  return false;
}


 /* Check that a date value is valid string YYYY-MM-DD, return the the epoch date */
function _fourD_analysis_validate_date( $d ) {
      $ymd = explode('-', $d );
      
      if( sizeof($ymd) != 3 || strlen($ymd[0]) != 4 || ((int)$ymd[1]) < 1 || ((int)$ymd[2]) < 1 ){
       return false;
       }
      else{
       $ymd[1] = str_replace('0', '', $ymd[1]);
       $ymd[2] = str_replace('0', '', $ymd[2]);
       if($ymd[1] < 10) $ymd[1] = "0" . $ymd[1]; // pad month with leading 0
       if($ymd[2] < 10) $ymd[2] = "0" . $ymd[2]; // pad day with leading 0
       $ymd[3] = $ymd[0].'-'.$ymd[1].'-'.$ymd[2];
       $epoch = strtotime( $ymd[3] );
       // fourD_analysis_debug('form_parts_validate; $ymd[3]: '. $ymd[3].';  epoch: '.$epoch );
       
       //if( !$time || $time <= time() ){ // Allow past dates? What about editing an old project?
       // $err = 'Date must be in the future.';
       if( $epoch > 1 ){
        return $epoch;
        }
       }
       return false;
}




















 /** Perform the Calculation of the combined Goal(s) weighting
   *
   * Formula: (G1*W1+G2*W2+G3*W3)
   *
   * G1 = Goal #1 Weighting
   * G2 = Goal #2 Weighting
   * G3 = Goal #3 Weighting
   *
   * W1 = Project Weighting of Goal #1
   * W2 = Project Weighting of Goal #2
   * W3 = Project Weighting of Goal #3
   *
   */
function _fourD_analysis_calculate_project_goal_weight( $nid, $uid=0, $goals, $project ) {
   //fourD_analysis_debug('_fourD_analysis_calculate_project_weight('.$nid.'); ');
   
   if( !count($goals) || !$project ){
    return 0.0;
    }
   
   $weight = 0;
   $indirectParents = array();
   if( isset($project['project_goal']) ){
    foreach( $goals as $gid=>$g ){

      if( isset($project['project_goal'][$gid]['rating']) ){
       $goal = fourD_analysis_goal_entry_get($gid, $uid);
       if($goal && isset($goal['goal_weight']) ){
        
        /* Direct parent and top-level are G1*W1 */
        if( _fourD_analysis_is_goal_applicable_to_project_input($nid, $g) ){
         $weight += ( $goal['goal_weight']*$project['project_goal'][$gid]['rating'] );
         }
        /* Indirect parents are simply (*W6*W7) - Note: not (G6*W6+G7*W7) */
        elseif( _fourD_analysis_is_goal_applicable_to_project($nid, $g) ){
         $indirectParents[$gid] = $goal['goal_weight'];
         }
        
        //fourD_analysis_debug('_fourD_analysis_calculate_project_weight; nid: '. $gid.'; W'.$gid.' = '.$goal['goal_weight'].';  G'.$gid.' = '.$project['project_goal'][$gid]['rating'].';  W'.$gid.'*G'.$gid.' = '.( $goal['goal_weight']*$project['project_goal'][$gid]['rating'] ). ';  Cumlative: '.$weight );
        }
       }
      }

     /* Finally, factor in any indirect parents */
     foreach($indirectParents as $nid=>$w){
       $weight = ($weight*$w);
       }
     }
   
   return $weight;
}


function _fourD_analysis_get_user_project_weights( $nid ) {
  
   $users = db_query("SELECT uid, assess_ability FROM {node_4dnetwork_analysis} WHERE nid = '%d'", $nid);
   while( $r = db_fetch_object($users) ) {
     $assess[$r->uid] = $r->assess_ability;
     }
  
}

 /** Perform the Calculation of the combined Prerequisite(s) weighting
   *
   * Formula: (G1*W1+G2*W2+G3*W3)
   *
   * G1 = Goal #1 Weighting
   * G2 = Goal #2 Weighting
   * G3 = Goal #3 Weighting
   *
   * W1 = Project Weighting of Goal #1
   * W2 = Project Weighting of Goal #2
   * W3 = Project Weighting of Goal #3
   *
   */
function _fourD_analysis_calculate_project_prerequisite_weight( $nid, $uid=0, $project=false ) {

   //$goals = _fourD_analysis_get_goals();
   if( !$project ) {
    $project = fourD_analysis_project_entry_get($nid, $uid);
    }
   
   if( !$project ){
    return 0.0;
    }
   
   $weight = 0;
   if( isset($project['project_prereq']) ){
    foreach( $project['project_prereq'] as $pid=>$pre){
      $weight += $pre['rating'];
      //fourD_analysis_debug('calculate_project_prerequisite_weight; pid: '. $pid.';  Weight: '.$pre['rating'].';  Cumlative: '.$weight );
      }
    }
   
   return $weight;
   
}

function _fourD_analysis_calculate_personal_project_weight( $uid, $nid ) {
   return _fourD_analysis_calculate_project_weight($nid, $uid);
}




/** Perform the Calculation of the overal Project weighting
  *
  * Revised: 2008.09.09
  *
  * --------------------------------------------------------------------------
  * 1. The personal weighting of person x of a project q is the following:
  * Wxq = ( ( (G1*PW1+G2*PW2+G3*PW3+G4*PW4+G5*PW5 * PW5subpra * PW5suprasupra...)+Pre)*Ps/T) *Dp*Ap*Ip / (deadline-date)
  *
  * Where:
  * G1  = Goal #1 Input (Goal tree level 1) 
  * G5  = Goal #5 Input (parent Goal of Project) 
  * PW1 = Personal Project Weighting of Goal #1  
  * Pre = Personal Weighting of any Prerequisite Projects 
  * T   = Personal Time (person years) 
  * Ps  = Personal Probability of Success 
  * Dp  = Personal Difference [will this project get done if I do not take part] 
  * Ap  = Personal Ability to complete the project 
  * Ip  = Personal Interest 
  * Deadline = (DD/365+MM/12+YYYY-2000) 
  *
  * Notes:
  * PW1 = the persons weighting of that goal1, not the communal average of goal1 and
  * Dp, Ap and Ip = the personal factors listed for D, A and I (i.e. the right hand column on the input table)
  *
  *
  * --------------------------------------------------------------------------
  * 2. 'project weighting' = 'subtotal' of project q is the following: 
  * WCq = (Wxq’*Axq + Wyq'*Ayq +... )/(Axq+Ayq...) 
  *
  * Where:
  * x and y are two people assessing the project 
  * Axq is the ability of person x to assess project q 
  * Wxq' = (((G1*CW1+G2*CW2+G3*CW3+G4*CW4+G5*CW5*CW5subpra*CW5suprasupra...)+Pre)*Ps/T)*Dt/(deadline-date) 
  * Dt = Overall Difference [will this project get done anyway, or not] 
  * CW1 = Community average Weighting of Goal #1  
  *
  * Notes: Wxq' is similar to Wxq but without the factors Ap and Ip and swapping
  *         Dp for Dt and using communal weightings of goals.
  *
  *
  * --------------------------------------------------------------------------
  *
  * 3. 'team project weighting' = 'communal' weighting of project q
  * Wtpq = (Wxq"*Axq+Wyq"*Ayq ... )/(Axq+Ayq...)
  * 
  * Where:
  * Wxq”= Wxq’*At*It 
  * At = Overall Ability of TEAM to complete the project 
  * It = Overall TEAM Interest 
  *
  * Notes: Wxq" is similar to Wxq’ but with the factors At and It. So, in full,
  *    Wxq”=(((G1*CW1+G2*CW2+G3*CW3+G4*CW4+G5*CW5*CW5subpra*CW5suprasupra...)+Pre)*Ps/T)*Dt*At*It/(deadline-date)
  *
  */
function _fourD_analysis_calculate_project_weight_internal( $nid, $uid=0, $getsubtotal=false ) {

   $goals = _fourD_analysis_get_goal_nodes($nid, 'all');

   $project = false;
   if( $uid ){
    $project = fourD_analysis_project_entry_get($nid, $uid);
    //fourD_analysis_debug('_fourD_analysis_calculate_project_weight('.$nid.', '.$uid.'); weighted_end / Personal ');
    if( $project['probability'] ){
     $project['difference'] = $project['personal_difference'];
     $project['ability'] = $project['personal_ability'];
     $project['interest'] = $project['personal_interest'];
     $project['deadline'] = $project['personal_deadline'];
     }
    //$communal = fourD_analysis_project_entry_get($nid);
    //if( $communal['probability'] ){
    // $project['probability'] = $communal['probability'];
    // $project['person_years'] = $communal['person_years'];
    // }
    }
   else{
    $project = fourD_analysis_project_entry_get($nid);
    if( $project['probability'] ){
     $project['difference'] = $project['communal_difference'];
     $project['ability'] = $project['communal_ability'];
     $project['interest'] = $project['communal_interest'];
     $project['deadline'] = $project['personal_deadline'];
     }
    }
  
   if( empty($goals) || !$project['probability'] ){
    //drupal_set_message('Incomplete input for Project weight calculation');
    return 0.0;
    }
   
   
   $goalWeights = _fourD_analysis_calculate_project_goal_weight( $nid, $uid, $goals, $project );
   $Pre = _fourD_analysis_calculate_project_prerequisite_weight( $nid, $uid, $project );
   
   
   /*  Deadline = (DD/365+MM/12+YYYY-2000)
    *  Should it be:
    *  Deadline = ( (DD/365) + (MM/12) + (YYYY-2000) )
    */
   $dateTime = getdate( $project['deadline'] );
   $deadline = (
               // $dateTime['yday']/365 + $dateTime['mon']/12 + $dateTime['year']-2000
               ($dateTime['yday']/365) + ($dateTime['mon']/12) + ($dateTime['year']-2000)
               );
   
   
   // Formula: (((G1*W1+G2*W2+G3*W3+G4*W4+G5*W5 * PW5subpra * PW5suprasupra )+Pre)*Ps/T)*D*A*I/(deadline-date)
   $WP = (($goalWeights+$Pre)*$project['probability']);
   $t1 = ( isset($project['person_years']) && $project['person_years'] > 0 ) ?
         ($WP/$project['person_years']) :
         $WP;
   
   
   if( $getsubtotal ){
    $weight = (
              $t1 *
              $project['difference']
              // *
              //$project['personal_ability'] *  // personal ability - not ability to assess
              //$project['personal_interest']
              ) / $deadline;
    }
   else{
    $weight = (
              $t1 *
              $project['difference'] *
              $project['ability'] *  // personal ability - not ability to assess
              $project['interest']
              ) / $deadline;
    }
   
   
  return $weight;
}


/** Perform the Calculation of the overal Project weighting
  *
  * @see Revision: 2008.09.09 above.
  */
function _fourD_analysis_calculate_project_weight( $nid, $uid=0, $getsubtotal=false ) {

   $weight = 0.0;
   
   /*  Factor in the Ability to Assess, by each user  */
   if( $uid ){
    $weight = _fourD_analysis_calculate_project_weight_internal( $nid, $uid, $getsubtotal );
    }
   
   else{
//    if( $getsubtotal ){
    /*  Factor in the Ability to Assess, by each user  */
    $usrs = db_query("SELECT uid, assess_ability FROM {node_4dnetwork_analysis} WHERE nid = '%d'", $nid);
    $users = array();
    while( $u = db_fetch_object($usrs) ) {
      $users[$u->uid] = $u->assess_ability;
      }
    
    // Formula: (Wxq"*Axq+Wyq"*Ayq ... )/(Axq+Ayq...)
    $ability_total = 0;
    foreach($users as $auid=>$ability){
      $ability_total += $ability;
      $weight += (_fourD_analysis_calculate_project_weight_internal( $nid, $auid, $getsubtotal ) * $ability );
      }
    
    if( $ability_total > 0 ){
     $weight = $weight/$ability_total;
     }
//     }
    }
  
  return $weight;
}






 /** Perform the Calculation of the overal Project weighting
   *
   * Formula: (((G1*W1+G2*W2+G3*W3+G4*W4+G5*W5)+Pre)*Ps/T)*D*A*I/(deadline-date)
   *
   * G1 = Goal #1 Input (Goal tree level 1)
   * G2 = Goal #2 Input (Goal tree level 1)
   * G3 = Goal #3 Input (Goal tree level 1)
   * G4 = Goal #4 Input (Goal tree level 1)
   * G5 = Goal #5 Input (parent Goal of Project)
   *
   * W1 = Project Weighting of Goal #1
   * W2 = Project Weighting of Goal #2
   * W3 = Project Weighting of Goal #3
   * W4 = Project Weighting of Goal #4
   * W5 = Project Weighting of Goal #5
   *
   * Pre = Overall Weighting of any Prerequisite Projects
   * T = Overall Time (person years)
   * Ps = Overall Probability of Success
   * D = Overall Difference
   * A = Overall Ability to complete the project
   * I = Overall Interest
   * Deadline = (DD/365+MM/12+YYYY-2000)
   *
   */
function _fourD_analysis_calculate_project_weight_old( $nid, $uid=0, $getsubtotal=false ) {
   //fourD_analysis_debug('_fourD_analysis_calculate_project_weight('.$nid.', '.$uid.'); ');
   
   /*  Things have changed several times here. */
   // $weight_style = 'weighted';
   // $weight_style = 'weighted_all';
   $weight_style = 'weighted_end';
   
   
   //$goals = _fourD_analysis_get_goals();
   $goals = _fourD_analysis_get_goal_nodes($nid, 'all');
   if( $weight_style == 'weighted_end' && $uid ){
    $project = fourD_analysis_project_entry_get($nid, $uid);
    //fourD_analysis_debug('_fourD_analysis_calculate_project_weight('.$nid.', '.$uid.'); weighted_end / Personal ');
    if( $project['probability'] ){
     $project['difference'] = $project['personal_difference'];
     $project['ability'] = $project['personal_ability'];
     $project['interest'] = $project['personal_interest'];
     $project['deadline'] = $project['personal_deadline'];
     }
    }
   else{
    $project = fourD_analysis_project_entry_get($nid);
    $project['difference'] = $project['communal_difference'];
    $project['ability'] = $project['communal_ability'];
    $project['interest'] = $project['communal_interest'];
    $project['deadline'] = $project['personal_deadline'];
    }
   
   if( empty($goals) || !$project['probability'] ){
    return 0.0;
    }
   
   fourD_analysis_debug('calculate_project_weight('.$nid.', '.$uid.'); $project: '. print_r($project, true) );
   
   $goalWeights = _fourD_analysis_calculate_project_goal_weight( $nid, $uid, $goals, $project );
   $Pre = _fourD_analysis_calculate_project_prerequisite_weight( $nid, $uid, $project );
   
   fourD_analysis_debug('calculate_project_weight('.$nid.', '.$uid.'); GoalWeight (G1*W1+G2*W2+G3*W3+G4*W4+G5*W5...): '. print_r($goalWeights, true) );
   fourD_analysis_debug('calculate_project_weight('.$nid.', '.$uid.'); PrequsiteWeight (P1+P2+P3...): '. print_r($Pre, true) );
   
   //fourD_analysis_debug('calculate_project_weight('.$nid.', '.$uid.'); Person Years: '.$project['person_years'] );
   
   $dateTime = getdate( $project['deadline'] );
   //fourD_analysis_debug('calculate_project_weight('.$nid.', '.$uid.'); Deadline Date: '.$project['deadline'].';  Date: '. print_r($dateTime, true) );
   
   

   /*  Deadline = (DD/365+MM/12+YYYY-2000)
    *  Should it be:
    *  Deadline = ( (DD/365) + (MM/12) + (YYYY-2000) )
    */
   $deadline = (
               // $dateTime['yday']/365 + $dateTime['mon']/12 + $dateTime['year']-2000
               ($dateTime['yday']/365) + ($dateTime['mon']/12) + ($dateTime['year']-2000)
               );
//    fourD_analysis_debug('calculate_project_weight('.$nid.', '.$uid.'); Deadline: '.$deadline );
   
   // Formula: (((G1*W1+G2*W2+G3*W3+G4*W4+G5*W5)+Pre)*Ps/T)*D*A*I/(deadline-date)
   $WP = (($goalWeights+$Pre)*$project['probability']);
   $t1 = ( isset($project['person_years']) && $project['person_years'] > 0 ) ?
         ($WP/$project['person_years']) :
         $WP;
   
   if( $getsubtotal ){
    $weight = (
              $t1 *
              $project['difference']
              // *
              //$project['personal_ability'] *  // personal ability - not ability to assess
              //$project['personal_interest']
              ) / $deadline;
    }
   else{
    $weight = (
              $t1 *
              $project['difference'] *
              $project['ability'] *  // personal ability - not ability to assess
              $project['interest']
              ) / $deadline;
    }
   
   /*  Finally, add the users' ability to assess the Project */
   if( $weight_style == 'weighted_all' ){
    $assesibleWeight = 0.0;
    if( $uid ){
     $users = db_query("SELECT uid, assess_ability FROM {node_4dnetwork_analysis} WHERE nid = '%d' AND uid = '%d'", $nid, $uid);
     }
    else{
     $users = db_query("SELECT uid, assess_ability FROM {node_4dnetwork_analysis} WHERE nid = '%d'", $nid);
     }
    while( $u = db_fetch_object($users) ) {
      $assesibleWeight += ($weight*$u->assess_ability);
      }
    return _fourD_analysis_round_decimal($assesibleWeight);
    }
   
   if( $weight_style == 'weighted_end' ){
    if( !$uid && $project['assess_ability'] > 0 ){
     $weights = array();
     $abilities = array();
     $users = db_query("SELECT uid, assess_ability FROM {node_4dnetwork_analysis} WHERE nid = '%d'", $nid);
     /* Add the users weights as such:
      * output weight =  W0 * (PW1*PA1 + PW2*PA2 + PW3*PA3 / PA1+PA2+PA3)
      *
      * W0 = Sub total weight
      * W1 = personal output weight (based on personal inputs only)
      * A1 = personal ability to assess
      */
     while( $u = db_fetch_object($users) ) {
       $abilities[$u->uid] = $u->assess_ability;
       $weights[$u->uid] = _fourD_analysis_calculate_personal_project_weight($u->uid, $nid);
       }
     
     $tot_weights = 0;
     $tot_abilities = 0;
     foreach($abilities as $uid=>$ab){
       $tot_abilities += $abilities[$uid];
       $tot_weights += ($weights[$uid]*$abilities[$uid]);
       }
     // fourD_analysis_debug('_fourD_analysis_calculate_project_weight('.$nid.', '.$uid.'); $tot_abilities: '.$tot_abilities.';  $tot_weights: '.$tot_weights );
     // nobody likes divide by zeros
     if( $tot_abilities > 0  ){
//       $weight = ($weight*($tot_weights/$project['assess_ability']) );
//       $weight = ($weight*($tot_weights/$tot_abilities) );
      // $subtotal = ($tot_weights/$tot_abilities);
      $weight = ($tot_weights/$tot_abilities);
      }
     }
    }

   return _fourD_analysis_round_decimal($weight);
}


/**  Same as _fourD_analysis_calculate_project_weight(), but does not include "ability" or "interest"  */
function _fourD_analysis_calculate_project_pre_weight( $nid, $uid=0 ) {
   //fourD_analysis_debug('_fourD_analysis_calculate_project_weight('.$nid.', '.$uid.'); ');
   
   return _fourD_analysis_calculate_project_weight( $nid, $uid, true );
}





