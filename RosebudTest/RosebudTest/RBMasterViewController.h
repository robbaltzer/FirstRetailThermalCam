//
//  RBMasterViewController.h
//  RosebudTest
//
//  Created by David Parker on 10/16/13.
//  Copyright (c) 2013 Novacoast. All rights reserved.
//

#import <UIKit/UIKit.h>

@class RBDetailViewController;

@interface RBMasterViewController : UITableViewController

@property (strong, nonatomic) RBDetailViewController *detailViewController;

@end
