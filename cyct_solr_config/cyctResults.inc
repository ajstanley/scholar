<?php

/*
 * contains methods to search solr and display results.  depends on Apache_Solr_Php client.
 */

module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrResults');

/**
 * Extention of IslandoraSolrResults for templating purposes only.
 * No changes have been made to the original class.
 */
class cyctSolrResults extends IslandoraSolrResults {

//default implementation of display results can be changed in the blocks config
  function displayResults($results) {
    global $islandora_query;

    global $base_url;
    $lines = array();

    foreach ($results->response->docs as $doc) {
      $resultsArray = array();
      foreach ($doc as $field => $value) {

        if (is_array($value)) {
          $value = implode(",  ", $value);
        }
        $resultsArray[$field] = $value;
      }
      $resultsArray['showAbstract'] = FALSE;
      if (preg_match('/PID:cyct/', $islandora_query)) {
        $resultsArray['showAbstract'] = TRUE;
      }
      $this->resultsArray = $resultsArray;
      $type = $resultsArray['refworks.rt'];
      switch ($type) {
        case "Journal Article":
        case "Magazine Article":
          $lines[] = array($this->articleDetails());
          break;
        case "Report":
          $lines[] = array($this->reportDetails());
          break;
        case "Book, Whole":
        case "Book, Edited":
          $lines[] = array($this->bookDetails());
      }
    }
    return theme_table(null, $lines);
  }

  function articleDetails() {

    $start = $this->resultsArray['refworks.sp'];
    $stop = $this->resultsArray['refworks.op'];
    $jf = $this->resultsArray['refworks.jf'];
    $volume = $this->resultsArray['refworks.vo'];
    $pageParts = array();
    if ($volume) {
      $pageParts[] = "Vol $volume";
    }
    $issue = $this->resultsArray['refworks.is'];
    if ($issue) {
      $pageParts[] = "Issue $issue";
    }
    $pages = "$start$stop";
    if ($pages) {
      $pages = "$pages";
    }
    if ($start && $stop) {
      $pages = "Pages $start - $stop";
    }
    if ($pages) {
      $pageParts[] = $pages;
    }
    $pageInfo = "";
    if ($pageParts) {
      $pageInfo = "<br />";
      foreach ($pageParts as $part) {
        $pageInfo .= "$part, ";
      }
      $pageInfo = preg_replace('/, $/', '', $pageInfo);
    }
    return $this->buildStub() . $jf . $pageInfo . "<br />" . $this->showFullText() . $this->showEditor() . "<br />" . $this->showAbstract();
////T1. A1 (FD YR). JF. Vol VO, Issue IS, SP - OP
  }

  function videoDetails() {
    global $base_url;
    $PID = $this->resultsArray['PID'];
    $title = $this->resultsArray['refworks.t1'] . " ";
    $author = $this->resultsArray['refworks.a1'] . " ";
    $yr = addSpace($this->resultsArray['refworks.yr']);
    $pp = addSpace($this->resultsArray['refworks.pp']);
    $pb = addSpace($this->resultsArray['refworks.pb']);
    return "$title$author$yr$pp$pb";
  }

  function reportDetails() {
    $t2 = $this->resultsArray['refworks.t2'];
    //$no = $this->resultsArray['refworks.no'];
    $ed = $this->resultsArray['refworks.ed'];
    $pp = $this->resultsArray['refworks.pp'];
    if ($pp) {
      $pp .= ':';
    }
    $pb = $this->resultsArray['refworks.pb'];

    $alternate = $t2 . $no . " ";
    if ($t2 && $no) {
      $alternate = "$t2/$no";
    }
    $edition = "$ed$pp ";
    if ($ed && $pp) {
      $edition = "$ed/$pp ";
    }
    return $this->buildStub() . $alternate . $edition . $pb . "<br />" . $this->showFullText() . $this->showEditor() . "<br />" . $this->showAbstract();
  }

  function bookDetails() {

    $pp = $this->resultsArray['pp'];
    $pb = $this->resultsArray['pb'];
    $pppb = $pb . $pp;
    if ($pb && $pp) {
      $pppb = "$pp:$pb ";
    }
    return $this->buildStub() . $pppb . "<br />" . $this->showFullText() . " " . $this->showEditor() . "<br />" . $this->showAbstract();
  }

  function buildStub() {
    global $base_url;

    $PID = $this->resultsArray['PID'];
    $escPID = str_replace(':', "\:", $PID);
    $title = $this->resultsArray['refworks.t1'] . " ";
    $titleTag = l($title, "$base_url/islandora/solr/search/PID:$escPID/-");
    $author = $this->resultsArray['refworks.a1'] . " ";
    $dateString = $this->resultsArray['refworks.fd'];
    if ($dateString) {
      $dateString .= " ";
    }
    $dateString .= $this->resultsArray['refworks.yr'];
    if ($dateString) {
      $dateString = "($dateString) ";
    }
    return "$titleTag<br />$author$dateString";
  }

  function showFullText() {
    global $base_url;
    $PID = $this->resultsArray['PID'];
    return "[" . l(t("Full Text"), "$base_url/fedora/repository/$PID/PDF") . "]";
  }

  function showEditor() {
    global $base_url;
    $PID = $this->resultsArray['PID'];
    $output = "[" . l(t("Edit Citation"), "$base_url/fedorair/ir_edit_refworks/$PID/") . "]";
    $output .= " [" . l(t("Technical Details"), "$base_url/fedora/repository/$PID/") . "]";
    return $output;
  }

  function showAbstract() {
    if ($this->resultsArray['refworks.k1']) {
      $links = '';
      $subjectArray = explode(', ', $this->resultsArray['refworks.k1']);
      foreach($subjectArray as $subject){
        $links .= l($subject, $base_url . '/islandora/solr/search/refworks.k1:"' .$subject . '"/-');
      }
      $output .= "<p><strong>Subject(s): </strong>" . $this->resultsArray['refworks.k1'] . "</p>";
    }


    if ($this->resultsArray['showAbstract']) {
      $output = "<br />";
      if ($this->resultsArray['refworks.k1']) {
        $output .= "<p><strong>Subject(s): </strong>" . $links . "</p>";
      }

      $output .= "<strong>Abstract:</strong><br />" . $this->resultsArray['refworks.ab'];
      return $output;
    }
  }

}

function addSpace($item) {
  if ($item) {
    $item = "$item ";
  }
  return $item;
}