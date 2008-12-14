<?php



 // ****************************************************************** //
 // **        project_view Inline Form                              ** //
 // **  Displays a List of Users' and their inputs, with editable   ** //
 // **  fields for each user to edit their own personal inputs      ** //
 // ****************************************************************** //



/**
 * Display a list of individual users' input on a Project - editable rows for the current users' input.
 */
function fourD_analysis_project_users_inputs_list_form($nid, $user_ids=false){
//   fourD_analysis_debug('users_inputs_list_form(); $nid: '.$nid.'; $user_ids: '.print_r($user_ids, true) );
//   drupal_set_message('users_inputs_list_form(); $nid: '.$nid.'; $user_ids: '.print_r($user_ids, true) );

  
  global $user;
  
  // Build the list of Prerequisite Inputs...
  $analcache = array();
  $goalInputs = array();
  $prereqInputs = array();
  foreach($user_ids as $i=>$uid){
    $analysis = fourD_analysis_project_entry_get($nid, $uid);
    if( $analysis && isset($analysis['person_years']) ){
     $analcache[$uid] = $analysis;
     
     if( isset($analysis['project_goal']) ){
      foreach( $analysis['project_goal'] as $gid=>$g ){
        if( !isset($goalInputs[$pid]) ){
         $goalInputs[$gid] = array('data' => $g['title'], 'title' => 'Goal: '.$g['title'] );
         }
        }
       }
     
     if( isset($analysis['project_prereq']) ){
      foreach( $analysis['project_prereq'] as $pid=>$pre ){
        if( !isset($prereqInputs[$pid]) ){
         $prereqInputs[$pid] = array('data' => $pre['title'], 'title' => 'Prerequisite: '.$pre['title'] );
         }
        }
       }
     
     }
    else{
     $analcache[$uid] = array();
     $analcache[$uid]['nid'] = $nid;
     }
    }
  
  $form = array();
  foreach($user_ids as $i=>$uid){
//     $analysis = fourD_analysis_project_entry_get($nid, $uid);
    $analysis = $analcache[$uid];
    if( is_array($analysis) ){
     if( $uid == $user->uid && user_access('submit project analysis', $user) ){
      /*  Populate any missing Goal inputs from this user  */
      foreach( $goalInputs as $gid=>$g ){
        if( !isset($analcache[$uid]['project_goal'][$gid]) ){
         $analcache[$uid]['project_goal'][$gid] = 0;
         }
        }
      
      /*  Populate any missing Prerequisite inputs from this user  */
      foreach( $prereqInputs as $pid=>$p ){
        if( !isset($analcache[$uid]['project_prereq'][$pid]) ){
         $analcache[$uid]['project_prereq'][$pid] = 0;
         }
        }
      
      $analcache[$uid]['nid'] = $nid; // In case there's no inputs yet.
      $parts = _fourD_analysis_project_form_parts( $analcache[$uid], true );
      $form['analysis'][$uid] = $parts['analysis'];
      }
     }
    }

  $form['analysis']['submit'] = array('#type' => 'submit', '#value' => t('Save Inputs') );
  $form['analysis']['#theme'] = 'fourD_analysis_project_users_inputs_list_form';
//   fourD_analysis_debug('users_inputs_list_form(); $nid: '.$nid.'; $user_ids: '.print_r($user_ids, true).'; $form: '.print_r($form, true) );
  return $form;
}





function fourD_analysis_project_users_inputs_list_form_validate( $formid, $form ){
  //drupal_set_message('users_inputs_list_form_validate(); $form: '.print_r($form, true) );
  
  if( !is_numeric(arg(1)) ){
   form_set_error('submit', 'Invalid node');
   return;
   }
  
  $node = node_load( arg(1) );
  if( !$node || !$node->nid ){
   form_set_error('submit', 'Invalid node');
   return;
   }
  
  global $user;
  foreach($form as $uid=>$values){
    if( is_numeric($uid) && $user->uid == $uid ){
     //fourD_analysis_debug('users_inputs_list_form_validate(); $uid: '.$uid );
     $user = user_load( array('uid'=>$uid) );
     if( !$user || !$user->uid ){
      form_set_error('submit', 'Invalid user');
      return;
      }
     
     $vals = fourD_analysis_project_entry_get(arg(1), $user->uid);
     $vals['nid'] = arg(1); // In case there's no inputs yet.
     $parts = _fourD_analysis_project_form_parts($vals);
     $validate['analysis'] = $values;
     _fourD_analysis_project_form_parts_validate( $validate, $parts );
     }
    }
}



function fourD_analysis_project_users_inputs_list_form_submit( $formid, $form){
  //drupal_set_message('users_inputs_list_form_submit(); nid: '.arg(1).'; $form: '.print_r($form, true) );
  
  $node = node_load( arg(1) );
  
  global $user;
  foreach($form as $uid=>$values){
    if( is_numeric($uid) && $user->uid == $uid ){
     //drupal_set_message('users_inputs_list_form_submit(); $uid: '.$uid );
     //fourD_analysis_debug('users_inputs_list_form_submit(); $uid: '.$uid );
     $user = user_load( array('uid'=>$uid) );
     if( !$user || !$user->uid ){
      //form_set_error('submit', 'Invalid user');
      drupal_set_message('Invalid user', 'error' );
      return;
      }
     
     foreach( $values['project_goal'] as $id => $value ) {
       $node->analysis['project_goal'][$id] = $value;
       }
     
     foreach( $values['project_prereq'] as $id => $value ) {
       $node->analysis['project_prereq'][$id] = $value;
       }
     
     foreach( $values['project'] as $id => $value ) {
       $node->analysis['project'][$id] = $value;
       }
     
     /* Auto Update/Insert DB function */
     //drupal_set_message('users_inputs_list_form_submit(); $uid: '.$uid.'; $node: '.print_r($node, true) );
     fourD_analysis_project_entry_update_insert($uid, $node);
     
     }
    }
}





/**
 * Coerce the form into a tidy horizontal table.
 */
function theme_fourD_analysis_project_users_inputs_list_form(&$form) {
//    fourD_analysis_debug('theme_fourD_analysis_project_users_inputs_list_form: $form: '.print_r($form, true) );
  
  global $user;
  $isEditable = 0;
  
  $nid = $form['#parameters'][1];
  $user_ids = $form['#parameters'][2];
  
  // Build the list of Goal Inputs...
  $goalInputs = array();
  $goals = _fourD_analysis_get_goal_nodes($nid);
  foreach( $goals as $gid=>$g ){
    //$goalInputs[$gid] = array('data' => $g['title'], 'title' => 'Goal: '.$g['title'], 'depth'=>$g['depth'],'class'=>'project-goal project-goal-l-' .$g['depth'] );
    $goalInputs[$gid] = array('data' => l(check_plain($g['title']), 'taxonomy/term/'.$g['tid'], array('title'=>'Goal: '.check_plain($g['title'])) ). ' [0-1]', 'title' => 'Goal: '.$g['title'], 'depth'=>$g['depth'],'class'=>'project-goal project-goal-l-' .$g['depth'] );
    }
  
  
  // Build the list of Prerequisite Inputs...
  $analcache = array();
  $prereqInputs = array();
  foreach($user_ids as $i=>$uid){
    $analysis = fourD_analysis_project_entry_get($nid, $uid);
    if( $analysis && isset($analysis['probability']) ){
     $analcache[$uid] = $analysis;
     
     /*  Owner can edit their own inputs */
     if( $uid == $user->uid ){
      $isEditable = 1;
      }
     
     if( isset($analysis['project_prereq']) ){
      foreach( $analysis['project_prereq'] as $pid=>$pre ){
        if( !isset($prereqInputs[$pid]) ){
         //$prereqInputs[$pid] = array('data' => $pre['title'], 'title' => 'Prerequisite: '.$pre['title'], 'class'=>'project-prereq' );
         $prereqInputs[$pid] = array('data' => l(check_plain($pre['title']), 'node/'.$pid, array('title'=>'Prerequisite: '.check_plain($pre['title'])) ). ' [0-1]', 'title' => 'Prerequisite: '.$pre['title'], 'class'=>'project-prereq' );
         }
        }
       }
     
     }
    }
  
  $rows = array();
  $maxCols = 0;
  $usrCols = 0;
  foreach($user_ids as $i=>$uid){
//     $analysis = fourD_analysis_project_entry_get($nid, $uid);
    
    /*  May be cached above ... */
    if( isset($analcache[$uid]) ){
     $analysis = $analcache[$uid];
     }
    else{
     $analysis = fourD_analysis_project_entry_get($nid, $uid);
     }
    
    //if( $analysis && isset($analysis['probability']) ){
//     if( $analysis ){
     
     //fourD_analysis_debug('theme_fourD_analysis_project_users_inputs_list_form: $form[analysis]['.$uid.']: '.print_r($form['analysis'][$uid], true) );
     
     $usrCols = 0;
     $usr = user_load( array('uid'=>$uid) );
     
     $row = array();
     
     /*  Add the Project Title Column  */
     $row[] = array('data' => l(check_plain($usr->name), 'user/'.$uid.'/projects', array('class'=>'project project-user', 'title'=>'View '.$usr->name.'s\' Projects') ), 'class'=>'project-user' );
     $usrCols++;
     
     /*  Owner can edit their own inputs */
     if( $uid == $user->uid ){
      
      /*  Add the Users Ability to assess Column First  */
      $tmp = drupal_render( $form['analysis'][$uid]['project']['assess_ability'] );
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      $row[] = array('data' => $tmp, 'class'=>'project project-assess_ability' );
      $usrCols++;
      
      foreach( $goalInputs as $gid=>$g ){
        $tmp = drupal_render( $form['analysis'][$uid]['project_goal'][$gid] );
        $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
        $row[] = array('data' => $tmp, 'class'=>'project-goal project-goal-l-' .$g['depth'] );
        $usrCols++;
        }
      
      foreach( $prereqInputs as $pid=>$data ){
        $tmp = drupal_render( $form['analysis'][$uid]['project_prereq'][$pid] );
        $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
        $row[] = array('data' => $tmp, 'class'=>'project-prereq' );
        $usrCols++;
        }
      
      $elems = array();
      if( is_array($form['analysis'][$uid]['project']) ) {
       foreach($form['analysis'][$uid]['project'] as $key => $elements) {
         if( is_array($elements) && strncmp($key, '#', 1) !== 0 ){
          if( $key != 'assess_ability' ){
           $tmp = drupal_render( $form['analysis'][$uid]['project'][$key] );
           $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
           // $row[] = array('data' => $tmp, 'class'=>'project' );
           $elems[$key] = $tmp;
           /*  Don't count these columns as they a colapsed into a single column (communal / personal) */
           if( $key != 'communal_difference' && $key != 'communal_ability' && $key != 'communal_interest' ){
            $usrCols++;
            }
           }
          }
         }
        }
      
      //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['assess_ability'] ), 'class'=>'project' );
      $row[] = array('data' => $elems['person_years'], 'class'=>'project' );
      $row[] = array('data' => $elems['probability'], 'class'=>'project' );
      
      $row[] = array('data' => $elems['communal_difference'] .' / '. $elems['personal_difference'], 'class'=>'project' );
      $row[] = array('data' => $elems['communal_ability'] .' / '. $elems['personal_ability'], 'class'=>'project' );
      $row[] = array('data' => $elems['communal_interest'] .' / '. $elems['personal_interest'], 'class'=>'project' );
      
      $row[] = array('data' => $elems['personal_deadline'], 'class'=>'project' );
      
      /*  Add the Users Weighted input Last  */
//       $row[] = array('data' => $tmp, 'class'=>'project project-weight' );
//       
//       $tmp = _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_pre_weight($nid));
//       $tmp .= ' / '. _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_weight($nid));
//       $row[] = array('data' => $tmp, 'class'=>'project project-weight' );
     

      $row[] = array('data' => _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_pre_weight($nid)), 'class'=>'project project-weight project-weight-subtotal' );
      $row[] = array('data' => _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_weight($nid)), 'class'=>'project project-weight project-weight-communal' );
      $row[] = array('data' => _fourD_analysis_trim_decimal(_fourD_analysis_calculate_personal_project_weight($uid, $nid)), 'class'=>'project project-weight project-weight-personal' );
      
      $usrCols+=3;
      }
     
     /*  Not Owner; can't edit these inputs */
     else{
      
      /*  Add the Users Ability to assess Column First  */
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['assess_ability'] ), 'class'=>'project project-assess_ability' );
      $usrCols++;
      
      foreach( $goalInputs as $gid=>$g ){
        $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['project_goal'][$gid]['rating'] ), 'class'=>'project-goal project-goal-l-' .$g['depth'] );
        $usrCols++;
        }
      
      foreach( $prereqInputs as $pid=>$data ){
        if( isset($analysis['project_prereq']) && isset($analysis['project_prereq'][$pid]) ){
         $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['project_prereq'][$pid]['rating'] ), 'class'=>'project-prereq' );
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project-prereq' );
         }
        $usrCols++;
        }
      
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['person_years'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['probability'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_difference'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_ability'] ), 'class'=>'project' );
      $row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['communal_interest'] ), 'class'=>'project' );
      //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['personal_difference'] ), 'class'=>'project' );
      //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['personal_ability'] ), 'class'=>'project' );
      //$row[] = array('data' => _fourD_analysis_trim_decimal( $analysis['personal_interest'] ), 'class'=>'project' );
      $row[] = array('data' => date('Y-m-d', $analysis['personal_deadline']), 'class'=>'project' );

      $row[] = array('data' => _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_pre_weight($nid)), 'class'=>'project project-weight project-weight-subtotal' );
      $row[] = array('data' => _fourD_analysis_trim_decimal(_fourD_analysis_calculate_project_weight($nid)), 'class'=>'project project-weight project-weight-communal' );

      //if( $isEditable ){
       $row[] = array('data' => _fourD_analysis_trim_decimal(_fourD_analysis_calculate_personal_project_weight($uid, $nid)), 'class'=>'project project-weight project-weight-personal' );
       //$row[] = array('data' => '-', 'class'=>'project project-weight project-weight-personal' );
       $usrCols++;
      // }
      $usrCols += 8;
      }
     
     $rows[] = $row;
     // Cache the max number of columns...
     if( $usrCols > $maxCols ){
      $maxCols = $usrCols;
      }
//      }
    }

  
  // Assemble the column headers...
//   $header = array();
//   $header[] = array('data' => 'User', 'class'=>'project project-user');
//   $header[] = array('data' => 'Assess Ability', 'class'=>'project', 'title' => t('Enter your personal ability to assess this project (ie: your personal level of expertise in regards to this project)'));
//   $header[] = array('data' => 'Person Years', 'class'=>'project');
//   $header[] = array('data' => 'Probability', 'class'=>'project');
//   $header[] = array('data' => 'Difference', 'class'=>'project');
//   $header[] = array('data' => 'Ability', 'class'=>'project');
//   $header[] = array('data' => 'Interest', 'class'=>'project');
//   $header[] = array('data' => 'Deadline', 'class'=>'project');
  
  $header = array();
  $header[] = _fourD_analysis_project_header_info('user');
  $header[] = _fourD_analysis_project_header_info('assess_ability');
  $header[] = _fourD_analysis_project_header_info('person_years');
  $header[] = _fourD_analysis_project_header_info('probability');
  //$header[] = _fourD_analysis_project_header_info('personal_difference');
  //$header[] = _fourD_analysis_project_header_info('personal_ability');
  //$header[] = _fourD_analysis_project_header_info('personal_interest');
  if( $isEditable ){
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
  
  
  $header[] = _fourD_analysis_project_header_info('weight_subtotal');
  //$header[] = _fourD_analysis_project_header_info('weight');
  $header[] = (count($rows) > 1) ?
                 array('data' => 'Weight', 'sort'=>'desc', 'field'=>'weight', 'class'=>'project project-weight') :
                 // array('data' => 'Weight', 'class'=>'project project-weight');
                 _fourD_analysis_project_header_info('weight_communal');
  
  //if( $isEditable ){
   $header[] = _fourD_analysis_project_header_info('weight_personal');
  // }
  
  // Add the extra Prerequisite Headers...
  $more = array_reverse($prereqInputs, true);
  foreach( $more as $pid=>$data ){
    //array_unshift($header, $data);
    array_splice($header, 2, 0, array($data));
    }
  
  // Add the extra Goal Headers...
  $more = array_reverse($goalInputs, true);
  foreach( $more as $gid=>$data ){
    //array_unshift($header, $data);
    array_splice($header, 2, 0, array($data));
    }
  
  $sort = ($_GET['sort'] == 'asc') ? 'asc' : 'desc';
  $rows = fourD_analysis_columnSort($rows, array(count($header)-1, $sort) );
  
  
  /*  Now tweak the weight output for scientific notation style
   *  - this must be done after the columnSort above.
   */
  foreach($rows as $i=>$row){
    $rows[$i][$maxCols-1]['data'] = _fourD_analysis_scientific_notation($row[$maxCols-1]['data']);
    $rows[$i][$maxCols-2]['data'] = _fourD_analysis_scientific_notation($row[$maxCols-2]['data']);
    if( $isEditable ){
     $rows[$i][$maxCols-3]['data'] = _fourD_analysis_scientific_notation($row[$maxCols-3]['data']);
     }
    }

  // Add the submit button in the bottom row....
  if( $isEditable ){
   $row = array();
   $form['analysis']['submit'] = array('#type' => 'submit', '#value' => t('Save') );
   $tmp = drupal_render( $form['analysis']['submit'] );
   $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
   $row[] = array('data' => $tmp, 'colspan'=>$maxCols );
   $rows[] = $row;
   }
  
  $title = 'Individual User analysis';
  $table = theme('table', $header, $rows, array(), $title);
  
  // Add the rest of the form parts...
  foreach($form as $key => $value) {
    if( is_array($value) ){
     if( $key != 'analysis' ){
      $table .= drupal_render( $value );
      }
     }
    }
  
  return $table;
}












 // ****************************************************************** //
 // **        user_view Inline Form                                 ** //
 // **  Displays a List of Projects' for the User with editable     ** //
 // **  fields for each field - for each project that they have     ** //
 // **  input analysis for                                          ** //
 // ****************************************************************** //



/**
 *  Arrgg, can't have the same form on the same page, more than once.
 *  This wrapper forces a unique ID for the Community Summary vs. User Input.
 *  This is more efficient than recreating the entire layout for each.
 */
function fourD_analysis_user_projects_inputs_list_summary_form($uid, $node_ids, $title){
  return fourD_analysis_user_projects_inputs_list_form($uid, $node_ids, $title);
}

/**
 *  Displays a List of Projects' for the User with editable
 *  fields for each field - for each project that they have
 *  input analysis for.
 */
function fourD_analysis_user_projects_inputs_list_form($uid, $node_ids, $title){
//   fourD_analysis_debug('user_projects_inputs_list_form(); $nid: '.$nid.'; $user_ids: '.print_r($user_ids, true) );
//   drupal_set_message('user_projects_inputs_list_form(); $nid: '.$nid.'; $user_ids: '.print_r($user_ids, true) );
  
  global $user;
//   $uid = $user->uid;
  $displayUser = user_load( array('uid'=>$uid) );
  
  $form = array();
  // fieldset - analysis
  $form['analysis'] = array(
//     '#title' => (
//                 (count($node_ids) > 1) ?
//                 (($uid == $user->uid) ? 'My ' : check_plain($displayUser->name) .'s '). t(' Projects') :
//                 (($uid == $user->uid) ? 'My ' : check_plain($displayUser->name) .'s '). t(' personal analysis inputs')
//                 ),
    '#title' => $title,
    '#type' => 'fieldset',
//     '#description' => t('Enter values for the various parameters. Explore the impact of ' .
//                         'different values. Play around a get a feel for it.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    );
  
  $form['analysis']['#display_user'] = $displayUser;
  $form['display_user'] = array(
  '#type' => 'value',
  '#value' => 'stub' // just a stub. It doesn't mean anything.
  );
  
  // Build the list of Prerequisite Inputs...
  $analcache = array();
  foreach($node_ids as $i=>$nid){
    $analysis = fourD_analysis_project_entry_get($nid, $uid);
    if( $analysis && isset($analysis['person_years']) ){
     $analcache[$nid] = $analysis;
     
     /*  Populate any missing Goal inputs  */
     $goals = _fourD_analysis_get_goal_nodes($nid);
     //fourD_analysis_debug('user_projects_inputs_list_form(); $nid: '.$nid.'; $goals: '.print_r($goals, true) );
     foreach( $goals as $gid=>$g ){
       if( !isset( $analcache[$nid]['analysis']['project_goal'][$gid] ) ){
        $analcache[$nid]['analysis']['project_goal'][$gid] = 0.0;
        }
       }
      
      //$analcache[$nid]['nid'] = $nid; // In case there's no inputs yet.
      $parts = _fourD_analysis_project_form_parts( $analcache[$nid], true );
      $form['analysis'][$nid] = //array( 'analysis'=>
                                  array(
                                   'project_goal'=>$parts['analysis']['project_goal'],
                                   'project_prereq'=>$parts['analysis']['project_prereq'],
                                   'project'=>$parts['analysis']['project']
                                   );
                                  //);
     }
    }
  
  
  /*  A user may only edit their own  */
  if( $uid == $user->uid && user_access('submit project analysis') ){
   $form['analysis']['submit'] = array('#type' => 'submit', '#value' => t('Save Inputs') );
   }
  $form['analysis']['#theme'] = 'fourD_analysis_user_projects_inputs_list_form_table';
  // fourD_analysis_debug('users_inputs_list_form(); $uid: '.$uid.'; $node_ids: '.print_r($node_ids, true).'; $form: '.print_r($form, true) );
  return $form;
}





function fourD_analysis_user_projects_inputs_list_form_validate( $formid, $form_values, $form ){
//   drupal_set_message('user_projects_inputs_list_form_validate(); $form_values: '.print_r($form_values, true) );
//   drupal_set_message('user_projects_inputs_list_form_validate(); $form: '.print_r($form, true) );
  
  global $user;
  $displayUser = $form['analysis']['#display_user'];
  form_set_value(array('#parents'=>array('display_user')), $displayUser ); // Save this for _submit
  //drupal_set_message('user_projects_inputs_list_form_validate(); $displayUser: '.print_r($displayUser, true) );
  
  foreach($form_values['analysis'] as $nid=>$values){
    
    if( is_numeric($nid) ){
     
     //fourD_analysis_debug('user_projects_inputs_list_form_validate(); $nid: '.$nid );
     $node = node_load( $nid );
     if( !$node || !$node->nid ){
      form_set_error('submit', 'Invalid Project ('.$nid.')');
      return;
      }
     
     $vals = fourD_analysis_project_entry_get($nid, $displayUser->uid);
     $vals['nid'] = $nid; // In case there's no inputs yet.
     $parts = _fourD_analysis_project_form_parts($vals);
     $validate['analysis'] = $values;
     _fourD_analysis_project_form_parts_validate( $validate, $parts );
     }
    }

}



function fourD_analysis_user_projects_inputs_list_form_submit( $formid, $form){
//   drupal_set_message('user_projects_inputs_list_form_submit(); nid: '.arg(1).'; $form: '.print_r($form, true) );
//   drupal_set_message('user_projects_inputs_list_form_submit(); $parts: '.print_r($parts, true) );
  
  global $user;
  $displayUser = $form['display_user'];
  //drupal_set_message('user_projects_inputs_list_form_submit(); $displayUser: '.print_r($displayUser, true) );
  
  foreach($form['analysis'] as $nid=>$values){
    if( is_numeric($nid) ){
     //fourD_analysis_debug('user_projects_inputs_list_form_submit(); $nid: '.$nid );
     $node = node_load( $nid );
     if( !$node || !$node->nid ){
      //form_set_error('submit', 'Invalid user');
      drupal_set_message('Invalid Project ('.$nid.')', 'error' );
      return;
      }
     
     foreach( $values['project_goal'] as $id => $value ) {
       $node->analysis['project_goal'][$id] = $value;
       }
     
     foreach( $values['project_prereq'] as $id => $value ) {
       $node->analysis['project_prereq'][$id] = $value;
       }
     
     foreach( $values['project'] as $id => $value ) {
       $node->analysis['project'][$id] = $value;
       }
     
     /* Auto Update/Insert DB function */
     //drupal_set_message('user_projects_inputs_list_form_submit(); $uid: '.$uid.'; $node: '.print_r($node, true) );
     fourD_analysis_project_entry_update_insert($displayUser->uid, $node);
     
     }
    }
}



/**
 * Coerce the form into a tidy table.
 */
function theme_fourD_analysis_user_projects_inputs_list_form_table(&$form){
   $disp = 'horizontal';
   //if( fourD_analysis_is_admin() )
   $disp = 'vertical';
   
   if( $disp == 'vertical' ){
    return theme_fourD_analysis_user_projects_inputs_list_form_table_vertical($form);
    }
   else if( $disp == 'horizontal' ){
    return theme_fourD_analysis_user_projects_inputs_list_form_table_horizontal($form);
    }
}




/**
 * Sort an array of nodes by weight
 */
// function fourD_analysis_rowSort($a, $b) {
//   return strnatcmp($a['weight'], $b['weight']);
// }
$globalMultisortRowVar = array();
function fourD_analysis_rowSort($recs, $cols=array('weight','desc') ) {
  //fourD_analysis_debug('fourD_analysis_columnSort; $cols: ' .$cols[0].','.$cols[1] );
     global $globalMultisortRowVar;
     $globalMultisortRowVar = $cols;
     uasort($recs, 'fourD_analysis_multiStrnatcmp_row');
     return ($recs);
 }
 
function fourD_analysis_multiStrnatcmp_row($a, $b) {
     global $globalMultisortRowVar;
     $cols = $globalMultisortRowVar;
     $i = 0;
     $result = 0;
     while ($result == 0 && $i < count($cols)) {
         $result = ($cols[$i + 1] == 'desc' ?
                          strnatcmp($b[$cols[$i]], $a[$cols[$i]]) :
                          $result = strnatcmp($a[$cols[$i]], $b[$cols[$i]])
                   );
         $i+=2;
     }
     return $result;
 }




/**
 * Coerce the form into a tidy vertical table.
 */
function theme_fourD_analysis_user_projects_inputs_list_form_table_vertical(&$form){
  // fourD_analysis_debug('theme_fourD_analysis_user_projects_inputs_list_form_table_vertical(); $form: '.print_r($form, true) );
  
  $order_by = 'weight';
  $sort = ($_GET['sort'] == 'asc') ? 'asc' : 'desc';

  $header = array(
              array('data' => 'Option', 'title' => 'Option')
              );
  
  global $user;
  $displayUser = $form['#display_user'];
  $uid = $displayUser->uid;
  $user_access_submit = user_access('submit project analysis');
  
  // Build the list of Prerequisite Inputs...
  $numProjects = 0;
  $projectInfo = array();
  $goalInputs = array();
  $prereqInputs = array();
  foreach($form as $nid=>$values){
   // fourD_analysis_debug('theme_user_projects_inputs_list_form; $nid: '.$nid );
   if( is_numeric($nid) ){
    $numProjects++;
    
    /*  Add the Project Column Header  */
    //$p = db_fetch_array(db_query('SELECT title FROM {node} WHERE nid = %d', $nid));
    $p = db_fetch_array(db_query("SELECT COUNT(*) as user_cnt, n.title FROM {node_4dnetwork_analysis} r INNER JOIN {node} n ON n.nid = r.nid WHERE r.nid = '%d' GROUP BY r.nid", $nid));

    $projectInfo[$nid] = array(
                           'title'=> $p['title'],
                           'user_cnt'=> $p['user_cnt'],
                           'weight_subtotal'=> _fourD_analysis_trim_decimal( _fourD_analysis_calculate_project_pre_weight($nid)),
                           'weight_communal'=> _fourD_analysis_trim_decimal( _fourD_analysis_calculate_project_weight($nid)),
                           'weight_personal'=> ($uid == $user->uid) ?
                               _fourD_analysis_trim_decimal( _fourD_analysis_calculate_personal_project_weight($user->uid, $nid)) :
                               0,
                           );
    
     $goals = _fourD_analysis_get_goal_nodes($nid);
     foreach( $goals as $gid=>$g ){
       if( !isset($goalInputs[$gid]) ){
        $goalInputs[$gid] = array(
                              'data' => $g['title'],
                              'title' => $g['title'],
                              'depth'=>$g['depth'],
                              'class'=>'project-goal project-goal-l-' .$g['depth'],
                              'tid'=> (module_exists('taxonomy_node') ? _taxonomy_node_get_tid_for_nid($gid) : 0 )
                              );
        }
       }
     
     // Build the list of Prerequisite Headers...
     $analysis = fourD_analysis_project_entry_get($nid, $displayUser->uid);
     $analysis['nid'] = $nid; // In case there's no inputs yet.
     if( $analysis && isset($analysis['probability']) ){
      if( isset($analysis['project_prereq']) ){
       foreach( $analysis['project_prereq'] as $pid=>$p ){
         if( !isset($prereqInputs[$pid]) ){
          $prereqInputs[$pid] = array(
                                'data' => $p['title'],
                                'title' => $p['title'],
                                'class'=>'project-prereq'
                                );
          }
         }
        }
      }
     }
    }
   
   if( count($projectInfo) ){
    $projectInfo = fourD_analysis_rowSort($projectInfo, array($order_by, $sort));
    }
   
   foreach($projectInfo as $nid=>$p){
     if( is_numeric($nid) ){
     $header[] = array(
                   'data' => l(check_plain($p['title']), 'node/'.$nid, array('title'=>'View Project: '.check_plain($p['title'])) ),
                   'title'=>'View Project: '.check_plain($p['title']), 'class'=>'project project-title'
                   );
      }
     }
  
  $tmp = '';
  $rows = array();

  /** Diplay the User count, and a link to Analytics if we're displaying Community Summaries here  */
  if( !$uid ){
   $row = array();
   $row[] = _fourD_analysis_project_header_info('user_cnt');
   //foreach($form as $nid=>$values){
   foreach($projectInfo as $nid=>$prInfo){
    $values = $form[$nid];
    if( is_numeric($nid) ){
      // fourD_analysis_debug('theme_user_projects_inputs_list_form; $goalInputs: '.$gid.'; $nid: '.$nid );
      $row[] = array('data' => $projectInfo[$nid]['user_cnt']. ' ('.l('view', 'project/'.$nid, array('title'=>'View '.check_plain($projectInfo[$nid]['title']).' Analytics') ).')', 'class'=>'project project-user_cnt' );
      }
     }
   $rows[] = $row;
   }
  
  /** Add the Users Ability to assess Row First  **/
  $row = array();
  $row[] = _fourD_analysis_project_header_info('assess_ability');
  //foreach($form as $nid=>$values){
  foreach($projectInfo as $nid=>$prInfo){
    $values = $form[$nid];
    // fourD_analysis_debug('theme_user_projects_inputs_list_form; $goalInputs: '.$gid.'; $nid: '.$nid );
    if( is_numeric($nid) ){
     if( $uid == $user->uid && $user_access_submit ){
      $tmp = drupal_render( $form[$nid]['project']['assess_ability'] );
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      }
     else{
      $tmp = $form[$nid]['project']['assess_ability']['#default_value'];
      }
     $row[] = array('data' => $tmp, 'class'=>'project project-assess_ability' );
     }
    }
   $rows[] = $row;
  
  
  
  /**  Add the Goal inputs - One row per Goal
    *  - only show the row if at least one Project has a value for that Goal.
    */
  foreach($goalInputs as $gid => $item) {
    if( is_numeric($gid) ) {
     //fourD_analysis_debug('theme_user_projects_inputs_list_form; $goalInputs: '.$gid );
     $row = array();
     
     /*  Add the Row Header  */
     $row[] = array(
              // 'data' => l(check_plain($item['title']), 'node/'.$gid, array('title'=>'Goal: '.check_plain($item['title'])) ),
                'data' => l(check_plain($item['title']), 'taxonomy/term/'.$item['tid'], array('title'=>'Goal: '.check_plain($item['title'])) ). ' [0-1]',
                'title'=>'Goal: '.check_plain($item['title']), 'class'=>'project-goal project-goal-l-' .$item['depth']
                );
      
      $row_has_value = 0;
      //($form as $nid=>$values){
      foreach($projectInfo as $nid=>$prInfo){
       // fourD_analysis_debug('theme_user_projects_inputs_list_form; $goalInputs: '.$gid.'; $nid: '.$nid );
       if( is_numeric($nid) ){
        
        $values = $form[$nid];
        if( isset($form[$nid]['project_goal']) ){
         if( is_array($form[$nid]['project_goal'][$gid]) ){
          if( $uid == $user->uid && $user_access_submit ){
           $tmp = drupal_render( $form[$nid]['project_goal'][$gid] );
           $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
           }
          else{
           $tmp = $form[$nid]['project_goal'][$gid]['#default_value'];
           }
          $row[] = array('data' => $tmp, 'class'=>'project-goal project-goal-l-' .$item['depth'] );
          $row_has_value = 1;
          }
         else{
          $row[] = array('data' => '-', 'class'=>'project-goal project-goal-l-' .$item['depth'] );
          }
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project-goal project-goal-l-' .$item['depth'] );
         }
       
       }
      }
     
     if( $row_has_value ){
      $rows[] = $row;
      }
     }
    }
  
  
  
  
  /**  Add the Prerequisite inputs - One row per Prerequisite
    *  - only show the row if at least one Project has a value for that Prerequisite.
    */
  foreach($prereqInputs as $pid => $item) {
    if( is_numeric($pid) ) {
     //fourD_analysis_debug('theme_user_projects_inputs_list_form; $prereqInputs: '.$pid );
     $row = array();
     
     /*  Add the Row Header  */
     $row[] = array(
                'data' => l(check_plain($item['title']), 'node/'.$pid, array('title'=>'Prerequisite: '.check_plain($item['title'])) ). ' [0-1]',
                'title'=>'Prerequisite: '.check_plain($item['title']), 'class'=>'project-prereq'
                );
      
      $row_has_value = 0;
      //foreach($form as $nid=>$values){
      foreach($projectInfo as $nid=>$prInfo){
       $values = $form[$nid];
       // fourD_analysis_debug('theme_user_projects_inputs_list_form; $prereqInputs: '.$pid.'; $nid: '.$nid );
       if( is_numeric($nid) ){
        
        if( isset($form[$nid]['project_prereq']) ){
         if( is_array($form[$nid]['project_prereq'][$pid]) ){
          if( $uid == $user->uid && $user_access_submit ){
           $tmp = drupal_render( $form[$nid]['project_prereq'][$pid] );
           $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
           }
          else{
           $tmp = $form[$nid]['project_prereq'][$pid]['#default_value'];
           }
          $row[] = array('data' => $tmp, 'class'=>'project-prereq' );
          $row_has_value = 1;
          }
         else{
          $row[] = array('data' => '-', 'class'=>'project-prereq' );
          }
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project-prereq' );
         }
       
       }
      }
     
     if( $row_has_value ){
      $rows[] = $row;
      }
     }
    }
  
  
//   $projectInputs = array('person_years', 'probability', 'communal_difference', 'communal_ability', 'communal_interest', 'personal_difference', 'personal_ability', 'personal_interest', 'personal_deadline');
  $projectInputs = array('person_years', 'probability', 'communal_difference', 'communal_ability', 'communal_interest', 'personal_deadline');
  
  /**  Add the Project inputs - One row per Input
    *  - only show the row if at least one Project has a value for that Input.
    */
  foreach($projectInputs as $i => $pid) {
     if( $uid == $user->uid && $user_access_submit ){
      if( $pid == 'communal_difference' ){
       $item = _fourD_analysis_project_header_info('communal_personal_difference');
       }
      elseif( $pid == 'communal_ability' ){
       $item = _fourD_analysis_project_header_info('communal_personal_ability');
       }
      elseif( $pid == 'communal_interest' ){
       $item = _fourD_analysis_project_header_info('communal_personal_interest');
       }
      else{
       $item = _fourD_analysis_project_header_info($pid);
       }
      }
     else{
      $item = _fourD_analysis_project_header_info($pid);
      }
     //fourD_analysis_debug('theme_user_projects_inputs_list_form; $projectInputs: '.$pid );
     $row = array();
     
     $row[] = $item;
      
      $row_has_value = 0;
      //foreach($form as $nid=>$values){
      foreach($projectInfo as $nid=>$prInfo){
       $values = $form[$nid];
       // fourD_analysis_debug('theme_user_projects_inputs_list_form; $projectInputs: '.$pid.'; $nid: '.$nid );
       if( is_numeric($nid) ){
        
        if( isset($form[$nid]['project']) ){
         if( is_array($form[$nid]['project'][$pid]) ){
          if( $uid == $user->uid && $user_access_submit ){
           $tmp = drupal_render( $form[$nid]['project'][$pid] );
           $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
           
           if( $pid == 'communal_difference' ){
            $tmp2 = drupal_render( $form[$nid]['project']['personal_difference'] );
            $tmp .= ' / '. preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp2);
            }
           elseif( $pid == 'communal_ability' ){
            $tmp2 = drupal_render( $form[$nid]['project']['personal_ability'] );
            $tmp .= ' / '. preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp2);
            }
           elseif( $pid == 'communal_interest' ){
            $tmp2 = drupal_render( $form[$nid]['project']['personal_interest'] );
            $tmp .= ' / '. preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp2);
            }
           
           }
          else{
           $tmp = $form[$nid]['project'][$pid]['#default_value'];
           }
          $row[] = array('data' => $tmp, 'class'=>'project' );
          $row_has_value = 1;
          }
         else{
          $row[] = array('data' => '-', 'class'=>'project' );
          }
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project' );
         }
       
       }
      }
     
     if( $row_has_value ){
      $rows[] = $row;
      }
    }
  
  $tosort = (($sort == 'asc') ? 'desc' : 'asc');
  $weight_communal_link = l('Weight  '.theme('tablesort_indicator', $tosort), $_GET['q'], array('class'=>'active'), 'sort='.$tosort.'&order='.$order_by, NULL, false, true );
  
  /** Add the Weight Row Last  **/
//   $row = array();
//   //$row[] = _fourD_analysis_project_header_info('weight');
//   $row[] = (count($projectInfo) > 1) ?
//                  array('data' => 'Weight', 'sort'=>'desc', 'field'=>'weight', 'class'=>'project project-weight') :
//                  array('data' => 'Weight', 'class'=>'project project-weight');
  $cell = _fourD_analysis_project_header_info('weight_communal');
  $cell['data'] = $weight_communal_link;
//   $row[] = $cell;
  
  $weight_rows = array();
  $weight_rows['weight_subtotal'][] = _fourD_analysis_project_header_info('weight_subtotal');
  $weight_rows['weight_communal'][] = $cell;
  if( $uid == $user->uid && $user_access_submit  ){
   $weight_rows['weight_personal'][] = _fourD_analysis_project_header_info('weight_personal');
   }


  foreach($projectInfo as $nid=>$prInfo){
    // fourD_analysis_debug('theme_user_projects_inputs_list_form; $goalInputs: '.$gid.'; $nid: '.$nid );
    if( is_numeric($nid) ){
     $values = $form[$nid];
//      if( $uid ){
//       $tmp = _fourD_analysis_trim_decimal( _fourD_analysis_calculate_personal_project_weight($uid, $nid));
//       //$tmp = _fourD_analysis_scientific_notation( _fourD_analysis_calculate_personal_project_weight($uid, $nid));
//       }
//      else{
//        $tmp = _fourD_analysis_trim_decimal( _fourD_analysis_calculate_project_weight($nid));
//        //$tmp = _fourD_analysis_scientific_notation( _fourD_analysis_calculate_project_weight($nid));
//       }
     
//      $tmp = $prInfo['weight'];
//      $tmp = _fourD_analysis_scientific_notation( $prInfo['communal_pre_weight'] );
//      $tmp .= ' / '. _fourD_analysis_scientific_notation( $prInfo['communal_weight'] );
//      $row[] = array('data' => $tmp, 'class'=>'project project-weight' );
     
     $tmp = _fourD_analysis_scientific_notation( $prInfo['weight_subtotal'] );
     $weight_rows['weight_subtotal'][] = array('data' => $tmp, 'class'=>'project project-weight project-weight-subtotal' );
     
     $tmp = _fourD_analysis_scientific_notation( $prInfo['weight_communal'] );
     $weight_rows['weight_communal'][] = array('data' => $tmp, 'class'=>'project project-weight project-weight-communal' );
     
     if( $uid == $user->uid && $user_access_submit ){
      $tmp = _fourD_analysis_scientific_notation( $prInfo['weight_personal'] );
      $weight_rows['weight_personal'][] = array('data' => $tmp, 'class'=>'project project-weight project-weight-personal' );
      }
     }
    }
//    $rows[] = $row;

  foreach($weight_rows as $row){
    $rows[] = $row;
    }
  
  /*  Add the submit button in the bottom row  */
  if( $uid == $user->uid && $user_access_submit ){
   $row = array();
   $form['submit'] = array('#type' => 'submit', '#value' => t('Save') );
   $tmp = drupal_render( $form['submit'] );
   $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
   $row[] = array('data' => $tmp, 'colspan'=>($numProjects+1) );
   $rows[] = $row;
   }

  return theme('table', $header, $rows);
}



/**
 * Coerce the form into a tidy horizontal table.
 */
function theme_fourD_analysis_user_projects_inputs_list_form_table_horizontal(&$form){
 // fourD_analysis_debug('theme_user_projects_inputs_list_form(); $form: '.print_r($form, true) );
//   fourD_analysis_debug('theme_user_projects_inputs_list_form(); $node_ids: '.print_r($form['#parameters'][1], true) );
//   drupal_set_message('user_projects_inputs_list_form(); $node_ids: '.print_r($form['#parameters'][1], true) );
  
  
  global $user;
  $displayUser = $form['#display_user'];
  $uid = $displayUser->uid;
  
  $user_access_submit = user_access('submit project analysis');
  
  $header[] = _fourD_analysis_project_header_info('project');
  // $header[] = _fourD_analysis_project_header_info('users');
  $header[] = _fourD_analysis_project_header_info('assess_ability');
  $header[] = _fourD_analysis_project_header_info('person_years');
  $header[] = _fourD_analysis_project_header_info('probability');
  $header[] = _fourD_analysis_project_header_info('communal_difference');
  $header[] = _fourD_analysis_project_header_info('communal_ability');
  $header[] = _fourD_analysis_project_header_info('communal_interest');
  $header[] = _fourD_analysis_project_header_info('personal_difference');
  $header[] = _fourD_analysis_project_header_info('personal_ability');
  $header[] = _fourD_analysis_project_header_info('personal_interest');
  $header[] = _fourD_analysis_project_header_info('personal_deadline');
  $header[] = _fourD_analysis_project_header_info('weight');
  
  $access_analytics = user_access('access analytics');
  if( $access_analytics ){ // Add the link to Detailed Project Analytics (user-by-user breakdown).
   $header[] = _fourD_analysis_project_header_info('analytics');
   }
  
  
  // Build the list of Prerequisite Inputs...
  $goalInputs = array();
  $prereqInputs = array();
  foreach($form as $nid=>$values){
   // fourD_analysis_debug('theme_user_projects_inputs_list_form; $nid: '.$nid );
   if( is_numeric($nid) ){
     $goals = _fourD_analysis_get_goal_nodes($nid);
     foreach( $goals as $gid=>$g ){
       if( !isset($goalInputs[$gid]) ){
        //$goalInputs[$gid] = array('data' => $g['title'], 'title' => 'Goal: '.$g['title'], 'depth'=>$g['depth'], 'class'=>'project-goal project-goal-l-' .$g['depth'] );
        $goalInputs[$gid] = array('data' => l(check_plain($g['title']), 'taxonomy/term/'.$g['tid'], array('title'=>'Goal: '.check_plain($g['title'])) ). ' [0-1]', 'title' => 'Goal: '.$g['title'], 'depth'=>$g['depth'], 'class'=>'project-goal project-goal-l-' .$g['depth'] );
        }
       }
     
     // Build the list of Prerequisite Headers...
     $analysis = fourD_analysis_project_entry_get($nid, $displayUser->uid);
     $analysis['nid'] = $nid; // In case there's no inputs yet.
     if( $analysis && isset($analysis['probability']) ){
      if( isset($analysis['project_prereq']) ){
       foreach( $analysis['project_prereq'] as $pid=>$p ){
         if( !isset($prereqInputs[$pid]) ){
          //$prereqInputs[$pid] = array('data' => $p['title'], 'title' => 'Prerequisite: '.$p['title'], 'class'=>'project-prereq' );
          $prereqInputs[$pid] = array('data' => l(check_plain($p['title']), 'node/'.$pid, array('title'=>'Prerequisite: '.check_plain($p['title'])) ). ' [0-1]', 'title' => 'Prerequisite: '.$p['title'], 'class'=>'project-prereq' );
          }
         }
        }
      }
     }
    }
  
  
  
  $tmp = '';
  $maxCols = 0;
  $rows = array();
  foreach($form as $nid=>$values){
   if( is_numeric($nid) ){
     $row = array();
     $nodeCols = 0;
     
     $t = db_fetch_array(db_query('SELECT title FROM {node} WHERE nid = %d', $nid));
     $row[] = array(
                // 'data' => check_plain($t['title']),
                'data' => l(check_plain($t['title']), 'node/'.$nid, array('title'=>'View Project: '.check_plain($t['title'])) ),
                'title'=>'View Project: '.check_plain($t['title']), 'class'=>'project project-title'
                );
     $nodeCols++;
     
     /*  Add the Users Ability to assess Column First  */
     if( $uid == $user->uid && $user_access_submit ){
      $tmp = drupal_render( $form[$nid]['project']['assess_ability'] );
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      }
     else{
      $tmp = $form[$nid]['project']['assess_ability']['#default_value'];
      }
     $row[] = array('data' => $tmp, 'class'=>'project project-assess_ability' );
     $nodeCols++;
     
     foreach( $goalInputs as $gid=>$g ){
       if( isset($form[$nid]['project_goal']) ){
        if( isset($form[$nid]['project_goal'][$gid]) ){
         if( $uid == $user->uid && $user_access_submit ){
          $tmp = drupal_render( $form[$nid]['project_goal'][$gid] );
          $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
          }
         else{
          $tmp = $form[$nid]['project_goal'][$gid]['#default_value'];
          }
         $row[] = array('data' => $tmp, 'class'=>'project-goal project-goal-l-' .$g['depth'] );
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project-goal project-goal-l-' .$g['depth'] );
         }
        }
       else{
        $row[] = array('data' => '-', 'class'=>'project-goal project-goal-l-' .$g['depth'] );
        }
       $nodeCols++;
       }
     
     foreach( $prereqInputs as $pid=>$data ){
       if( isset($analysis['project_prereq']) ){
        if( isset($analysis['project_prereq'][$pid]) ){
         if( $uid == $user->uid && $user_access_submit ){
          $tmp = drupal_render( $form[$nid]['project_prereq'][$pid] );
          $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
          }
         else{
          $tmp = $form[$nid]['project_prereq'][$pid]['#default_value'];
          }
         $row[] = array('data' => $tmp, 'class'=>'project-prereq' );
         }
        else{
         $row[] = array('data' => '-', 'class'=>'project-prereq' );
         }
        }
       else{
        $row[] = array('data' => '-', 'class'=>'project-prereq' );
        }
       $nodeCols++;
       }
      
      if( is_array($form[$nid]['project']) ) {
       foreach($form[$nid]['project'] as $key => $elements) {
         if( is_array($elements) && strncmp($key, '#', 1) !== 0 ){
          if( $key != 'assess_ability' ){
           if( $uid == $user->uid && $user_access_submit ){
            $tmp = drupal_render( $form[$nid]['project'][$key] );
            $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
            }
           else{
            $tmp = $form[$nid]['project'][$key]['#default_value'];
            }
           $row[] = array('data' => $tmp, 'class'=>'project' );
           $nodeCols++;
           }
          }
         }
        }
      
      /*  Add the Users Weighted input Last  */
      $row[] = array('data' => _fourD_analysis_trim_decimal( _fourD_analysis_calculate_personal_project_weight($uid, $nid)), 'class'=>'project project-weight' );
      $nodeCols++;
      
      if( $access_analytics ){
       $row[] = array('data' => l('View', 'project/'.$nid, array('title'=>'View '.check_plain($t['title']).' Analytics') ), 'class'=>'project project-analytics' );
       $nodeCols++;
       }
      
      $rows[] = $row;
      // Cache the max number of columns...
      if( $nodeCols > $maxCols ){
       $maxCols = $nodeCols;
       }
      }
     }
  
  
  /*  Add the submit button in the bottom row  */
  if( $uid == $user->uid && $user_access_submit ){
   $row = array();
   $form['submit'] = array('#type' => 'submit', '#value' => t('Save') );
   $tmp = drupal_render( $form['submit'] );
   $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
   $row[] = array('data' => $tmp, 'colspan'=>$maxCols );
   $rows[] = $row;
   }
  
  /*  Add the extra Prerequisite Headers  */
  $more = array_reverse($prereqInputs, true);
  foreach( $more as $pid=>$data ){
    //array_unshift($header, $data);
    array_splice($header, 2, 0, array($data));
    }
  
  /*  Add the extra Goal Headers  */
  $more = array_reverse($goalInputs, true);
  foreach( $more as $gid=>$data ){
    //array_unshift($header, $data);
    array_splice($header, 2, 0, array($data));
    }
  
  
//   $title = 'My Individual Project analysis';
//   return theme('table', $header, $rows, array(), $title);
  return theme('table', $header, $rows);
  
}










 // ****************************************************************** //
 // ************            node_view Inline Form          *********** //
 // ****************************************************************** //


/**
 * Add the User Analysis Info to the /node/XX pages.
 * - This form allows users to add/edit their personal inputs.
 * - Each user has their own analysis inputs, editable inline.
 */
function fourD_analysis_project_analysis_inline_form($node){
// function fourD_analysis_project_analysis_inline_form($node, $usr=false, $editable=true){
//   fourD_analysis_debug('add_inline_form(); $node: '.$node->nid.'; $usr: '.$usr.'; $editable: '.$editable  );
  
//   if( !$usr ){
   global $user;
   $usr = $user;
//    }
  
  if( $usr && user_access('access analysis', $usr) ){
   
   $values = fourD_analysis_project_entry_get($node->nid, $usr->uid);
   $values['nid'] = $node->nid; // In case there's no inputs yet.
   $form = _fourD_analysis_project_form_parts( $values, true );
   
   $form['analysis']['#title'] = 'Project Analysis';
   $form['analysis']['#description'] = null;
   
   $form['analysis']['#theme'] = 'fourD_analysis_project_analysis_inline_form_table';
   
   if( user_access('submit project analysis', $usr) ){
    $form['analysis']['submit'] = array('#type' => 'submit', '#value' => t('Save') );
    }
   
   //fourD_analysis_debug('_fourD_analysis_add_user_analysis; $node->user_analysis: ' .$node->user_analysis );
   
   return $form;
   }
}

function fourD_analysis_project_analysis_inline_form_validate( $formid, $form ){
//   fourD_analysis_debug('add_inline_form_validate; $form: '.print_r($form, true) );
  if( !is_numeric(arg(1)) ){
   form_set_error('title', 'Invalid node');
   return;
   }
//    _fourD_analysis_project_form_parts_validate( $form );
   global $user;
   $values = fourD_analysis_project_entry_get(arg(1), $user->uid);
   $values['nid'] = arg(1); // In case there's no inputs yet.
   $parts = _fourD_analysis_project_form_parts($values);
   _fourD_analysis_project_form_parts_validate( $form, $parts );
}

function fourD_analysis_project_analysis_inline_form_submit( $formid, $form){
//   fourD_analysis_debug('add_inline_form_submit nid: '.arg(1).'; $form: '.print_r($form, true) );
  
  $node = node_load( arg(1) );
  
  // Add the Goal form parts to the node, for saving below...
//   $parts = _fourD_analysis_project_form_parts();
  global $user;
  $values = fourD_analysis_project_entry_get($node->nid, $user->uid);
  $values['nid'] = $node->nid; // In case there's no inputs yet.
  $parts = _fourD_analysis_project_form_parts($values);
  
  foreach( $parts['analysis']['project_goal'] as $key => $value ) {
     if( is_array($value) ){
     $node->analysis['project_goal'][$key] = $form['analysis']['project_goal'][$key];
     // fourD_analysis_debug('fourD_analysis_project_analysis_inline_form_submit;  $key: '.$key.';  $value: '.$node->analysis['goal'][$key] );
     }
    }
  
  // Add the Prerequisite form parts to the node, for saving below...
  foreach( $parts['analysis']['project_prereq'] as $key => $value ) {
    // fourD_analysis_debug('fourD_analysis_project_analysis_inline_form_submit;  $key: '.$key.';  $value: '.print_r($form['analysis']['project_prereq'][$key],true) );
    if( $key == 'add' ){
     $node->analysis['project_prereq'][$key] = $form['analysis']['project_prereq'][$key];
     }
    else if( is_array($value) && is_numeric($key) && is_numeric($form['analysis']['project_prereq'][$key]) ){
     $node->analysis['project_prereq'][$key] = $form['analysis']['project_prereq'][$key];
     }
    }
  
  // Add the Project form parts to the node, for saving below...
  foreach( $parts['analysis']['project'] as $key => $value ) {
     if( is_array($value) ){
     $node->analysis['project'][$key] = $form['analysis']['project'][$key];
     // fourD_analysis_debug('fourD_analysis_project_analysis_inline_form_submit;  $key: '.$key.';  $value: '.$node->analysis['prereq'][$key] );
     }
    }
  
  /* Auto Update/Insert DB function */
//   fourD_analysis_debug('add_inline_form_submit $user->uid: '.$user->uid.'; $node: '.print_r($node, true) );
  fourD_analysis_project_entry_update_insert($user->uid, $node);
  
}



/**
 * Coerce the form into a tidy table.
 *
 * Used at the bottom of Analysis items for adding/updating
 * personal analysis inputs.
 */
function theme_fourD_analysis_project_analysis_inline_form_table(&$form){
   // fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table');
  
  /* Dispay the table Horizontaly */
  // return theme_fourD_analysis_project_analysis_inline_form_table_horizontal($form);
  
  /* Dispay the table Vertically */
  return theme_fourD_analysis_project_analysis_inline_form_table_vertical($form);
}




/**
 * Coerce the form into a tidy horizontal table.
 *
 * Used at the bottom of Analysis items for adding/updating
 * personal analysis inputs.
 */
function theme_fourD_analysis_project_analysis_inline_form_table_horizontal(&$form){
   // fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table_horizontal');
   
   $header = array(
              array('data' => 'Person Years',        'title' => 'Person Years'),
              array('data' => 'Probability',         'title' => 'Probability'),
              array('data' => 'Personal Difference', 'title' => 'Personal Difference'),
              array('data' => 'Personal Ability',    'title' => 'Personal Ability'),
              array('data' => 'Personal Interest',   'title' => 'Personal Interest'),
              array('data' => 'Personal Deadline',   'title' => 'Personal Deadline'),
              array('data' => '')
              );
   
   //$goals = array_reverse( _fourD_analysis_get_goals() );
   $goals = array_reverse( _fourD_analysis_get_goal_nodes() );
   foreach( $goals as $gid=>$g ){
     array_unshift( $header,  array('data' => $g['title'], 'title' => 'Goal: '.$g['title'].'' ) );
     }
  
  $rows = array(array());
  $submit = '';
  $other = '';
//   $parts = _fourD_analysis_project_form_parts();
  global $user;
  if( is_numeric(arg(1)) ){
   $values = fourD_analysis_project_entry_get(arg(1), $user->uid);
   $values['nid'] = arg(1); // In case there's no inputs yet.
   }
  $parts = _fourD_analysis_project_form_parts($values);
  
  foreach($form as $fkey => $fvalue) {
    if( is_array($fvalue) ){
     
     if( $fkey == 'submit' ){ // Save the submit button for the last column
      $tmp = drupal_render($form[$fkey]);
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      $submit = $tmp;
      }
     
     else if( $fkey == 'project_goal' || $fkey == 'project' ){
      $type = $fkey;
      foreach($form[$type] as $key => $value) {
         if( is_array($value) ){
          // fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table;  $type: '.$type.';  $key: '.$key.';  $parts[\'analysis\']['.$type.']['.$key.']: '.print_r($parts['analysis'][$type][$key], true).';  $value: '.print_r($value, true) );
          if( isset($parts['analysis'][$type][$key]) ){
           //fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table;  $type: '.$type.';  $key: '.$key.';  $value: '.print_r($value, true) );
           $tmp = drupal_render( $form[$type][$key] );
           $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
           $rows[0][] = array('data' => $tmp );
           }
          }
         }
      }
     
     else{
      $other .= drupal_render($form[$fkey]) . "\n";
      }
     }
    }


  $rows[0][] = array('data' => $submit );

//   return theme_table($header, $rows) . $other;
  return theme('table', $header, $rows) . $other;
}







/**
 * Coerce the form into a tidy vertical table.
 *
 * Used at the bottom of Analysis items for adding/updating
 * personal analysis inputs.
 */
function theme_fourD_analysis_project_analysis_inline_form_table_vertical(&$form){
//    fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table_vertical();  $form: '.print_r($form, true));
  
  global $user;
  $user_access_submit = user_access('submit project analysis');
  
  $header = array(
              array('data' => 'Option', 'title' => 'Option'),
              array('data' => 'Community Analysis',  'title' => 'Community Analysis')
              );
  if($user_access_submit) $header[] = array('data' => 'My Input',  'title' => 'My Input');
  
  $nid = 0;
  $projectAnalysis = array();
  if( arg(0) == 'node' && is_numeric(arg(1)) ){
   $nid = arg(1);
   $projectAnalysis = fourD_analysis_project_entry_get($nid);
   $projectAnalysis['nid'] = $nid; // In case there's no inputs yet.
   }
  else{
   return 'Invalid Node!';
   }
  
  $parts = _fourD_analysis_project_form_parts( $projectAnalysis );
  
  $rows = array();
  $prereqAddRow = array();
  $assessAbilityRow = array();
  
  /*  Add the Goal inputs  */
//   foreach($parts['analysis']['project_goal'] as $id => $item) {
//     if( is_numeric($id) ) {
  $goals = _fourD_analysis_get_goal_nodes($nid);
  foreach( $goals as $id=>$g ){
    if( is_numeric($id) ){
     $item = $parts['analysis']['project_goal'][$id];
     $row = array(
                 //array('data'=>$item['#title'], 'title'=> 'Goal: '.$item['#title'], 'class'=>'project-goal project-goal-l-' .$g['depth'] ),
                 array('data'=>l(check_plain($g['title']), 'taxonomy/term/'.$g['tid'], array('title'=>'Goal: '.check_plain($g['title'])) ). ' [0-1]', 'title'=> 'Goal: '.$item['#title'], 'class'=>'project-goal project-goal-l-' .$g['depth'] ),
                 array('data'=>$item['#default_value'], 'title'=> 'Goal: '.$item['#title'], 'class'=>'project-goal project-goal-l-' .$g['depth'] )
                 );
     if($user_access_submit){
      $tmp = drupal_render( $form['project_goal'][$id] );
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      $row[] = array('data'=>$tmp, 'title'=> 'Goal: '.$item['#title'], 'class'=>'project-goal project-goal-l-' .$g['depth'] );
      }
     $rows[] = $row;
     }
    }
  
  /*  Add extra Prerequisite inputs  */
  $prereqKeys = array();
  foreach($parts['analysis']['project_prereq'] as $id => $item) {
    // fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table_vertical();  project_prereq; $id: '.$id.';  $item: '.print_r($item, true));

    if( is_numeric($id) || $id == 'add' ) {
     if( $id == 'add' ){
      //$rows[] = array(
      $prereqAddRow = array(
                  array('data'=>$item['#title'], 'title'=> $item['#title'], 'class'=>'project-prereq project-prereq-add' ),
                  array('data'=>'', 'title'=> $item['#title'], 'class'=>'project-prereq project-prereq-add' )
                  );
      if($user_access_submit){
       $tmp = drupal_render( $form['project_prereq'][$id] );
       $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
       $prereqAddRow[] = array('data'=>$tmp, 'title'=> $item['#title'], 'class'=>'project-prereq project-prereq-add' );
       }
      }
     else{
     $prereqKeys[] = $id;
     $row = array(
                 //array('data'=>$item['#title'], 'title'=> 'Prerequisite Project: '.$item['#title'], 'class'=>'project-prereq' ),
                 array('data'=>l(check_plain($item['#title']), 'node/'.$id, array('title'=>'Prerequisite: '.check_plain($item['#title'])) ). ' [0-1]', 'title'=> $item['#title'], 'class'=>'project-prereq' ),
                 array('data'=>$item['#default_value'], 'title'=> 'Prerequisite Project: '.$item['#title'], 'class'=>'project-prereq' )
                 );
     if($user_access_submit){
      $tmp = drupal_render( $form['project_prereq'][$id] );
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      $row[] =  array('data'=>$tmp, 'title'=> 'Prerequisite Project: '.$item['#title'], 'class'=>'project-prereq' );
      }
     $rows[] = $row;
      }
     }
    }

  $projectInputs = array('assess_ability', 'person_years', 'probability', 'communal_difference', 'communal_ability', 'communal_interest', 'personal_deadline');

  /*  Add the Project inputs  */
//   foreach($parts['analysis']['project'] as $key => $item) {
  foreach($projectInputs as $i => $pid) {
//     if( is_array($item) ) {
     
     $row = array();
     if( $user_access_submit ){
      if( $pid == 'communal_difference' ){
       $row[] = _fourD_analysis_project_header_info('communal_personal_difference');
       }
      elseif( $pid == 'communal_ability' ){
       $row[] = _fourD_analysis_project_header_info('communal_personal_ability');
       }
      elseif( $pid == 'communal_interest' ){
       $row[] = _fourD_analysis_project_header_info('communal_personal_interest');
       }
      else{
       $row[] = _fourD_analysis_project_header_info($pid);
       }
      }
     else{
      $row[] = _fourD_analysis_project_header_info($pid);
      }
     
     $row[] = array('data'=>$form['project'][$pid]['#default_value'], 'class'=>'project project-'.$pid );
     
     if( $user_access_submit ){
      $tmp = drupal_render( $form['project'][$pid] );
      $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
      
      if( $pid == 'communal_difference' ){
       $tmp2 = drupal_render( $form['project']['personal_difference'] );
       $tmp .= ' / '. preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp2);
       }
      elseif( $pid == 'communal_ability' ){
       $tmp2 = drupal_render( $form['project']['personal_ability'] );
       $tmp .= ' / '. preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp2);
       }
      elseif( $pid == 'communal_interest' ){
       $tmp2 = drupal_render( $form['project']['personal_interest'] );
       $tmp .= ' / '. preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp2);
       }
      
      $row[] = array('data' => $tmp, 'class'=>'project project-'.$pid );
      }
     if( $pid == 'assess_ability' ){
      $assessAbilityRow = $row;
      }
     else{
      $rows[] = $row;
      }
//      }
    }
   
   
   /*  Unshift the Ability to Assess to the Top most row  */
   //$rows[] = $assessAbilityRow;
   array_unshift($rows, $assessAbilityRow );
   
   /*  Add the Row displaying the Project Weight  */
   $row = array(
            _fourD_analysis_project_header_info('weight_subtotal_communal'),
            array('data' =>
                    _fourD_analysis_scientific_notation( _fourD_analysis_calculate_project_pre_weight($nid) ) .' / '.
                    _fourD_analysis_scientific_notation( _fourD_analysis_calculate_project_weight($nid) ),
                  'class'=>'project project-weight' )
            );
   
   if($user_access_submit){
    //$row[] = array('data' => _fourD_analysis_trim_decimal(_fourD_analysis_calculate_personal_project_weight($user->uid, $nid)), 'class'=>'project project-weight' );
    $row[] = array('data' => _fourD_analysis_scientific_notation(_fourD_analysis_calculate_personal_project_weight($user->uid, $nid)), 'class'=>'project project-weight' );
    }
   $rows[] = $row;
   
   
   if($user_access_submit) {
    /*  Add the Row to add Prequisites Projects  */
    $prereqAddRow[1]['data'] = implode(',', $prereqKeys);
    $rows[] = $prereqAddRow;
   
   /*  Add the Row and a submit button  */
    $tmp = drupal_render($form['submit']);
    $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
    $rows[] = array(
               array('data' => ''),
               array('data' => ''),
               array('data' => $tmp )
               );
    }

//   return theme_table($header, $rows) . $other;
  return theme('table', $header, $rows) . $other;
}


/**
 * Coerce the form into a tidy vertical table.
 *
 * Used at the bottom of Projects view for adding/updating
 * personal analysis inputs.
 *
 *
 *  deprecated - 2008-07-10
 *  New version consolidates the Community and Personal analysis into one table.
 */
function theme_fourD_analysis_project_analysis_inline_form_table_vertical_old(&$form){
//    fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table_vertical();  $form: '.print_r($form, true));
   
   $header = array(
              array('data' => 'Option', 'title' => 'Option'),
              array('data' => 'My Input',  'title' => 'My Input'),
//               array('data' => 'Person Years',        'title' => 'Person Years'),
//               array('data' => 'Probability',         'title' => 'Probability'),
//               array('data' => 'Personal Difference', 'title' => 'Personal Difference'),
//               array('data' => 'Personal Ability',    'title' => 'Personal Ability'),
//               array('data' => 'Personal Interest',   'title' => 'Personal Interest'),
//               array('data' => 'Personal Deadline',   'title' => 'Personal Deadline'),
//               array('data' => '')
              );
  
  
//   $parts = _fourD_analysis_project_form_parts();
  global $user;
  if( is_numeric(arg(1)) ){
   $values = fourD_analysis_project_entry_get(arg(1), $user->uid);
   $values['nid'] = arg(1); // In case there's no inputs yet.
   }
  $parts = _fourD_analysis_project_form_parts($values);
  
  $rows = array();
  $goals = array();
  $prereqs = array();
  $prereqAdd = array();
  foreach($form as $fkey => $fvalue) {
    if( is_array($fvalue) ){
     if( $fkey != 'submit' ){ // Save the submit button for the last row
      
      if( $fkey == 'project_goal' || $fkey == 'project_prereq' || $fkey == 'project' ){
       $type = $fkey;
       foreach($form[$type] as $key => $value) {
          if( is_array($value) ){
           // fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table;  $type: '.$type.';  $key: '.$key.';  $parts[\'analysis\']['.$type.']['.$key.']: '.print_r($parts['analysis'][$type][$key], true).';  $value: '.print_r($value, true) );
           if( isset($parts['analysis'][$type][$key]) ){
            //fourD_analysis_debug('theme_fourD_analysis_project_analysis_inline_form_table;  $type: '.$type.';  $key: '.$key.';  $value: '.print_r($value, true) );
            $tmp = drupal_render( $form[$type][$key] );
            $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
            if( $fkey == 'project_prereq' ){
             if( $key == 'add' ){
              $prereqAdd = array(
                        array('data' => $form[$type][$key]['#title'], 'title'=> $form[$type][$key]['#title'], 'class'=>'project-prereq project-prereq-add' ),
                        array('data' => $tmp, 'class'=>'project-prereq project-prereq-add' )
                        );
              }
             else{
              $prereqs[] = array(
                        array('data' => $form[$type][$key]['#title'], 'title'=> 'Prerequisite Project: '.$form[$type][$key]['#title'], 'class'=>'project-prereq' ),
                        array('data' => $tmp, 'class'=>'project-prereq' )
                        );
              }
             }
            else if( $fkey == 'project_goal' ){
             $goals[] = array(
                       array('data' => $form[$type][$key]['#title'], 'title'=> 'Goal: '.$form[$type][$key]['#title'], 'class'=>'project-goal' ),
                       array('data' => $tmp, 'class'=>'project-goal' )
                       );
             }
            else if( $fkey == 'project' ){
             $rows[] = array(
                       //array('data' => $form[$type][$key]['#title'], 'title'=> $form[$type][$key]['#description'], 'class'=>'project' ),
                       _fourD_analysis_project_header_info($key),
                       array('data' => $tmp, 'class'=>'project' )
                       );
             }
            }
           }
          }
       }
      
      }
     }
    }

   // Add the Goal Rows...
   $tmp = array_merge($goals, $prereqs);
   
   // Add the Prerequisite Rows...
   $rows = array_merge($tmp, $rows);
   $rows[] = $prereqAdd;

   // Add the Row and submit button...
   $tmp = drupal_render($form['submit']);
   $tmp = preg_replace('/^.+<input([^>]+)>.+$/is', '<input$1>', $tmp);
   $rows[] = array(
              array('data' => ''),
              array('data' => $tmp )
              );

//   return theme_table($header, $rows) . $other;
  return theme('table', $header, $rows) . $other;
}




