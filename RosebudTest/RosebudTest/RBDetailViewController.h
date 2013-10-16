//
//  RBDetailViewController.h
//  RosebudTest
//
//  Created by David Parker on 10/16/13.
//  Copyright (c) 2013 Novacoast. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface RBDetailViewController : UIViewController <UISplitViewControllerDelegate>

@property (strong, nonatomic) id detailItem;

@property (weak, nonatomic) IBOutlet UILabel *detailDescriptionLabel;
@end
