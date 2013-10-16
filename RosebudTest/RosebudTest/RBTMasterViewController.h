//
//  RBTMasterViewController.h
//  RosebudTest
//
//  Created by David Parker on 10/16/13.
//  Copyright (c) 2013 Novacoast. All rights reserved.
//

#import <UIKit/UIKit.h>

@class RBTDetailViewController;

@interface RBTMasterViewController : UITableViewController

@property (strong, nonatomic) RBTDetailViewController *detailViewController;

@end
