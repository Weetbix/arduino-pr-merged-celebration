## An octocat which dances when you merge a PR!

![octocat](https://user-images.githubusercontent.com/492636/45980861-6c80d080-c053-11e8-9bfd-b6aedd6263fb.gif)

## About

Whenever you merge a PR, Octocat will do a randomly generated dance as a reward for your hard work.

This is an IoT project built using an ESP8266 and the github API. It parses your github event feed every 3 or so seconds and if a PR is merged, generates some dance patterns for a connected servo. The servo is connected to the octocat skeleton which is made out of left over cardboard packaging found around my flat.

Features:
- Dances are randomly generated
- Sends an ETag header so you will not encouter rate-limit issues with the Github API
- Runs over wifi
- Uses WiFIiManager to connect to wifi, and provide a github token in a safe manner

## Pictures

![img_20180924_225254_bokeh](https://user-images.githubusercontent.com/492636/45981627-06497d00-c056-11e8-8343-a848b17761bf.jpg)
![img_20180924_230501](https://user-images.githubusercontent.com/492636/45981629-06497d00-c056-11e8-82c2-525b83090357.jpg)
![img_20180924_230322](https://user-images.githubusercontent.com/492636/45981628-06497d00-c056-11e8-9eef-30ff1f9380ca.jpg)

## Create your own:

You can create a puppet and upload this project straight onto your own ESP8266 and you may get decent results.


### Create a Github Personal Access Token

You must create a Github Personal Access token with these permissions:

```
notifications
user - read:user
```

User read permissions are used to fetch your username from the token, and notification permission is used to fetch the notification stream for your user which includes the PR merge events.

You can create tokens here: https://github.com/settings/tokens


### First boot

On first boot, we don't know what wifi or github token you want to use, so the arduino will host its own access point with these parameters:

```
SSID: PR-Celebration
PASS: mergemaster
```

You should connect to this access point, which will then ask you to configure the WIFI the ESP should connect to, and your github token.

On subsequent boots, this information is remembered and does not need to be re-entered.

Note: You can reset these settings by hitting the reset button twice within 3 seconds.


## Extra configuration

Below you fill find the parameters you probably want to configure if you have setup this on your own ESP8266 or wifi connected arduino, and hooked up a servo to your puppet.

In `main.cpp`:

Var Name|Description|Default
-|-|-
`ACCESS_POINT_NAME`|SSID to use for the access point when configuration is needed|PR-Celebration
`ACCESS_POINT_PW`|Password to use for the configuration AP|mergemaster
`GITHUB_TIME_BETWEEN_QUERIES`|Time in milliseconds between queries to the github notification stream|3000
`SERVO_CONTROL_PIN`|The ID for the pin which your servo control is connected to| Digital 3
`SERVO_MIN_ANGLE`|Your puppet will likely have different extent requirements, so with these settings you can configuration the minimum and maximum value which the servo should ever go to. Dances will be generated within these params|65
`SERVO_MIN_ANGLE`|Your puppet will likely have different extent requirements, so with these settings you can configuration the minimum and maximum value which the servo should ever go to. Dances will be generated within these params|115
