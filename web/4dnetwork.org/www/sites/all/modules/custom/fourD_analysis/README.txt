Readme
------

 This module provides Project Analysis for 4dnetwork.org.



/**********************************************
 PROJECT ANALYSIS EQUATION - LATEST (2008.09.09)
 **********************************************/

1. The personal weighting of person x of a project q is the following: 
Wxq = ( ( (G1*PW1+G2*PW2+G3*PW3+G4*PW4+G5*PW5 * PW5subpra * PW5suprasupra...)+Pre)*Ps/T) *Dp*Ap*Ip / (deadline-date)
 
Where: 
  * G1 = Goal #1 Input (Goal tree level 1) 
  * G5 = Goal #5 Input (parent Goal of Project) 
  * PW1 = Personal Project Weighting of Goal #1  
  * Pre = Overall Weighting of any Prerequisite Projects 
  * T = Overall Time (person years) 
  * Ps = Overall Probability of Success 
  * Dp = Personal Difference [will this project get done if I do not take part] 
  * Ap = Personal Ability to complete the project 
  * Ip = Personal Interest 
  * Deadline = (DD/365+MM/12+YYYY-2000) 
 
[Note PW1=the persons weighting of that goal1, not the communal average of goal1 and Dp, Ap and Ip = the personal factors listed for D, A and I (i.e. the right hand column on the input table)]  
 




2. 'project weighting' = 'subtotal' of project q is the following: 
WCq = (Wxq’*Axq + Wyq'*Ayq +... )/(Axq+Ayq...) 
 
Where: 
  * x and y are two people assessing the project 
  * Axq is the ability of person x to assess project q 
  * Wxq' = (((G1*CW1+G2*CW2+G3*CW3+G4*CW4+G5*CW5*CW5subpra*CW5suprasupra...)+Pre)*Ps/T)*Dt/(deadline-date) 
  * Dt = Overall Difference [will this project get done anyway, or not] 
  * CW1 = Community average Weighting of Goal #1  
 
[Notes: Wxq' is similar to Wxq but without the factors Ap and Ip and swapping Dp for Dt and using communal weightings of goals.]  
 




3. 'team project weighting' = 'communal' weighting of project q 
Wtpq = (Wxq"*Axq+Wyq"*Ayq ... )/(Axq+Ayq...) 
4W06 
Where: 
  * Wxq”= Wxq’*At*It 
  * At = Overall Ability of TEAM to complete the project 
  * It = Overall TEAM Interest 
 
[Notes: Wxq" is similar to Wxq’ but with the factors At and It. So, in full, Wxq”=(((G1*CW1+G2*CW2+G3*CW3+G4*CW4+G5*CW5*CW5subpra*CW5suprasupra...)+Pre)*Ps/T)*Dt*At*It/(deadline-date)]  
 






























/**********************************************
 PROJECT ANALYSIS EQUATION
 **********************************************/

the project analysis equation is currently:

W = GW (dG * I * P * T * A * Ex * En)/(Yr * $)

where the parameters are defined as follows:

GW [0,1] =  global weighting, a parameter to reflect the priority level of
	    this project with respect to the collective high level
	    strategy (note: strategy part of the site to be
	    implemented in the future; for now this factor is not counted)
dG [0,1] =  the incremental (delta) difference the project makes in achieving
	    your goals
I [0,1] =   your interest in doing the project
P [0,1] =   the probability of the project succeeding
T [0,1] =   the timeliness of the project (now versus later)
A [0,1] =   your comparative advantage in working on the project
	    (skills you bring, would it get done without you, etc.)
Ex [0,1] =  externalities (social, environmental). 
En [0,1] =  the enabling role that this project will have for other
	    projects or for your skillset, experience, or contacts
	    (which in turn contributes to your ability to better
	    implement future projects)
HH [yrs] =  estimated time needed to implement the project
$ [$] =     estimated money needed to implement the project
W [1/] =   the final weighting of the project, as calculated from the
	    above values. 


/**********************************************
 DATA STRUCTURES
 **********************************************/

the project analysis module implements a new database table with the
following information (in addition to the standard node information
such author, comments, last updated times, etc.

note that the 'project name' and 'decscription' fields make use of the
standard node 'title' and 'body' fields respectively. these listed
below are additional:

table: projectanalysis
FIELD NAME            	DATA TYPE
----------------------|----------
global_weighting	DOUBLE
del_goals		DOUBLE
interest		DOUBLE
probability		DOUBLE
timeliness		DOUBLE
advantage		DOUBLE
externality		DOUBLE
enabling		DOUBLE
time			DOUBLE
money			DOUBLE
weighting		DOUBLE

/**********************************************
 Structure of a "Create Project" Node:
 **********************************************/

Project Name ('title')
Summary
Description ('body)
Project Analysis Parameters

