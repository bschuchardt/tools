#!/usr/bin/python
#
# A script to poll some jenkins builds and
# report the unique test failures in them. This
# script can also compare with a JIRA query to see if 
# the test failures occur in that query
#
# TODO - add support for creating issues. The script could do it through the rest
# API, or just give someone a URL to create a bug like this:
# https://issues.apache.org/jira//secure/CreateIssueDetails!init.jspa?pid=12318420&issuetype=1&summary=new%20failure&description=this%20is%20a%20test

from jira import JIRA
import requests
import json
import argparse
import sys
import re


def main(argv): 

  requests.packages.urllib3.disable_warnings()
  parser = argparse.ArgumentParser(description='Find all failed tests, starting with the minimum build number')
  parser.add_argument('-start', help='Build number to start from', required=True, type=int)
  parser.add_argument('-job', help='Build number to start from', required=True)
  parser.add_argument('-url', help='Jenkins URL. Default is https://brazil.gemstone.com:8080/', required=False, default='https://brazil.gemstone.com:8080/')
  parser.add_argument('-jira', help='Jira server to check for tickets. Possible values are internal or apache ', required=False, default='apache', choices=['apache', 'internal'])
        

  args = parser.parse_args();


#  jobName = 'GemFire_develop_open_CC';
  newFailures = {};
  oldFailures = {};
  counts = {};

  startJob = args.start;

  lastJob = getLastJob(args)

  jiraServer = args.jira;

  jiras,jira_descriptions = getJiraTickets(jiraServer)

  print "Last job was " + str(lastJob);

  for jobNum in range(startJob , lastJob + 1):
    try:
      checkJob(args, jobNum, newFailures, oldFailures, counts, jiras, jira_descriptions)
      print "Parsed " + str(jobNum)
    except ValueError:
      print "Could not parse the data for job " + str(jobNum);

  
  print "------------------------------------------------------------"
  print "Previously seen failures"
  print "------------------------------------------------------------"
  for test in sorted(oldFailures, key=lambda testName: counts[testName]):
    print '{:2d} {:10} {}'.format(counts[test], oldFailures[test], test)

  print "\n------------------------------------------------------------"
  print "New failures"
  print "------------------------------------------------------------"
  for test in sorted(newFailures, key=lambda testName: counts[testName]):
    print '{:2d} {}'.format(counts[test], test)
    print newFailures[test]
    print '\n'

def getLastJob(args):
  url = args.url + '/job/' + args.job + '/'
  r = requests.get(url + 'api/json?pretty=true&tree=lastCompletedBuild[number]', verify=False)
  data = r.json();
  return data['lastCompletedBuild']['number']
    

def checkJob(args, jobNum, newFailures, oldFailures, counts, jiras, jira_descriptions):
  url = args.url + '/job/' + args.job + '/' + str(jobNum) + '/testReport/';
  r = requests.get(url + 'api/json?pretty=true&tree=suites[cases[className,name,status]]', verify=False)
  data = r.json();
  for suite in data['suites']:
    for case in suite['cases']:
      status = case['status']
      if(status != 'SKIPPED' and status != 'FIXED' and status != 'PASSED'):
        className = case['className']
        methodName = case['name']
        shortClassName = getShortName(className);
        testName =  shortClassName + "." + methodName
        #This ugly mess replaces the last . in the class name with a /
        testURL =  getTestUrl(className)


        foundClass = findJira(shortClassName, methodName, jiras, jira_descriptions)

        if(counts.has_key(testName)):
          counts[testName] = counts[testName] + 1
        else:
          counts[testName] = 1

        if(foundClass != ''): 
          oldFailures[testName] = '{:3d} {}'.format(jobNum, foundClass)
        else:
          newFailures[testName] = url + testURL + '/' + cleanChars(case['name'])


def getJiraTickets(jiraServer):
  if(jiraServer == 'apache'):
    jira = JIRA('https://issues.apache.org/jira/')
    jira_ci_issues = jira.search_issues('project = GEODE AND labels = CI', maxResults=False)
  elif(jiraServer == 'internal'):
    jira = JIRA('https://jira.eng.pivotal.io')
    jira_ci_issues = jira.search_issues('project = GEM AND labels = CI', maxResults=False)

  else:
    raise Exception('Bad jira server ' + jiraServer)

  jira_summaries = {}
  jira_descriptions = {}
  for issue in jira_ci_issues:
    jira_summaries[issue.fields.summary] = '{:10} {:11} {:.10}'.format(issue.key, issue.fields.status.name, issue.fields.updated)
    jira_descriptions[issue.fields.summary] = issue.fields.description

  return (jira_summaries, jira_descriptions)

def getShortName(className):
  substrings = className.split('.')
  return substrings[len(substrings) - 1]

def getTestUrl(className):
  substrings = className.split('.')
  className = '.'.join(substrings[0:len(substrings) -1])
  testName = substrings[len(substrings) - 1]
  return className + "/" + testName
  
def cleanChars(text):
  return re.sub('[^0-9A-Za-z]', '_', text)

def findJira(testName, methodName, jiras, jira_descriptions):
  for issue in jiras:
    if(testName in issue and methodName in issue):
      return jiras[issue]
    if(testName in jira_descriptions[issue] and methodName in jira_descriptions[issue]):
      return jiras[issue]

  return ''

def usage():
  print 'polljenkins.py -j GEODE-XXX (Geode issue)'



if __name__ == "__main__":
   main(sys.argv[1:])


