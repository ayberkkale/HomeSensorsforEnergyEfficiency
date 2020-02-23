//
//  ViewController.swift
//  MAKMA Home
//
//  Created by M. Ayberk Kale on 2.10.2017.
//  Copyright © 2017 MAKMA. All rights reserved.
//

import UIKit
import CoreLocation
import FirebaseDatabase

class ViewController: UIViewController, CLLocationManagerDelegate {
    
    
    
    //text field
    
    
    @IBOutlet weak var TemperatureValue: UITextField!
    
    @IBOutlet weak var TempSetChanged: UIButton!
    
    @IBAction func TempSet(_ sender: Any) {
        
        
        var ref: FIRDatabaseReference?
        
        ref = FIRDatabase.database().reference()
        
        let string = NSString(string: TemperatureValue!.text!)
        
       
        
        ref?.child("manualTempSet").setValue(string.doubleValue)
            TemperatureValue.resignFirstResponder()
        
            
        
        
       
    }
    
    @IBOutlet weak var LightCondition: UISwitch!
    
    
    
    
    @IBAction func LightConditionChanged(_ sender: UISwitch) {
        var ref: FIRDatabaseReference?
        
        
   
        ref = FIRDatabase.database().reference()
        
       
        if sender.isOn {
            
            ref?.child("LightCondition").setValue(Bool(true))
            
            
            
            
        }else{
            ref?.child("LightCondition").setValue(Bool(false))
            
            
        }
        
    }
    
    

    
    
    @IBOutlet weak var AirConCondition: UISwitch!
    
    @IBAction func AirConditionChanged(_ sender: UISwitch) {
        var ref: FIRDatabaseReference?
        
        ref = FIRDatabase.database().reference()
        if sender.isOn {
            
            ref?.child("AirConCondition").setValue(Bool(true))
            
            
        }else{
            ref?.child("AirConCondition").setValue(Bool(false))
            
            
        }
    }
    
  
    @IBOutlet weak var WindowsConCondition: UISwitch!
    
    @IBAction func WindowsCondChanged(_ sender: UISwitch) {
        
        var ref: FIRDatabaseReference?
        
        ref = FIRDatabase.database().reference()
        if sender.isOn {
            
            ref?.child("WindowsCondition").setValue(Bool(true))
            
            
        }else{
            ref?.child("WindowsCondition").setValue(Bool(false))
            
            
        }
    }
    
    @IBAction func ManualModeButton(_ sender: UISwitch) {
        
        var ref: FIRDatabaseReference?
        
        ref = FIRDatabase.database().reference()
        if sender.isOn {
            
            ref?.child("isManualControlActive").setValue(Bool(true))
          //  AirConCondition.isEnabled = true
            
            
         //   WindowsConCondition.isEnabled = true
            
        //    LightCondition.isEnabled = true
            
           TempSetChanged.isEnabled = true
        //    TemperatureValue.isHidden=true
            
  
            
        }else{
            ref?.child("isManualControlActive").setValue(Bool(false))
            
            
            //AirConCondition.isEnabled = false
           //WindowsConCondition.isEnabled = false
          //  LightCondition.isEnabled = false
            TempSetChanged.isEnabled = true
           
          //  TemperatureValue.isHidden=false
           
            
            
        }
        
        
    }
    
    
    
    // Used to start getting the users location
    let locationManager = CLLocationManager()
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        
        // For use when the app is open & in the background
        locationManager.requestAlwaysAuthorization()
        
        // For use when the app is open
        //locationManager.requestWhenInUseAuthorization()
        
        // If location services is enabled get the users location
        if CLLocationManager.locationServicesEnabled() {
            locationManager.delegate = self
            locationManager.desiredAccuracy = kCLLocationAccuracyBest // You can change the locaiton accuary here.
            locationManager.startUpdatingLocation()
        }
    }
    
    // Print out the location to the console
    func locationManager(_ manager: CLLocationManager, didUpdateLocations locations: [CLLocation]) {
        if let location = locations.first {
            //print(location.coordinate)
            
            //location to firebaseserver
            
            var ref: FIRDatabaseReference?
            
            ref = FIRDatabase.database().reference()
            
            let when = DispatchTime.now() + 6 // change 6 to desired number of seconds
            DispatchQueue.main.asyncAfter(deadline: when) {
                
                //ref?.child("location/latitude").childByAutoId().setValue(String(location.coordinate.latitude))
                // ref?.child("location/longitude").childByAutoId().setValue(String(location.coordinate.longitude))
                ref?.child("location/latitude").setValue(Double(location.coordinate.latitude))
                ref?.child("location/longitude").setValue(Double(location.coordinate.longitude))
            }
            
            
        }
    }
    
    // If we have been deined access give the user the option to change it
    func locationManager(_ manager: CLLocationManager, didChangeAuthorization status: CLAuthorizationStatus) {
        if(status == CLAuthorizationStatus.denied) {
            showLocationDisabledPopUp()
        }
    }
    
    // Show the popup to the user if we have been deined access
    func showLocationDisabledPopUp() {
        let alertController = UIAlertController(title: "Background Location Access Disabled",
                                                message: "In order to use homekit hardware we need your location",
                                                preferredStyle: .alert)
        
        let cancelAction = UIAlertAction(title: "Cancel", style: .cancel, handler: nil)
        alertController.addAction(cancelAction)
        
        let openAction = UIAlertAction(title: "Open Settings", style: .default) { (action) in
            if let url = URL(string: UIApplicationOpenSettingsURLString) {
                UIApplication.shared.open(url, options: [:], completionHandler: nil)
            }
        }
        alertController.addAction(openAction)
        
        self.present(alertController, animated: true, completion: nil)
    }
    
}

