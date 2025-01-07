#![allow(non_snake_case)]
use serde::{Deserialize, Serialize};
use lazy_static::lazy_static;

const FILE_LOCATION: &str = "../../.env";

#[derive(Clone, Debug)]
pub struct Env {
    DBLP_RECORDS_LENGTH: usize,
    SURNAMES_LENGTH: usize,
}

impl Env {
    pub fn get_DBLP_RECORDS_LENGTH(&self) -> usize {
        self.DBLP_RECORDS_LENGTH
    }

    pub fn get_SURNAMES_LENGTH(&self) -> usize {
        self.SURNAMES_LENGTH
    }
}

lazy_static! {
    static ref ENV: Env = get_env_variables();
}

// Access the ENV anywhere in your application
pub fn get_ENV() -> &'static Env {
    &ENV
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Data {
    #[serde(rename = "author's name")]
    pub authors_name: String,
    pub title: String,
    #[serde(rename = "gap of years")]
    pub gap_of_years: Vec<i32>,
    #[serde(rename = "year of release")]
    pub year_of_release: i32,
    #[serde(rename = "DBLP_Record")]
    pub dblp_record: String,
    #[serde(rename = "Awards")]
    pub awards: i32,
    pub kind: String,
    #[serde(rename = "co-author")]
    pub co_author: Vec<String>,
    pub surname: String,
}

pub fn get_env_variables() -> Env {
    // read file line by line
    let file = std::fs::read_to_string(FILE_LOCATION).unwrap();
    let lines: Vec<&str> = file.split('\n').collect();
    return Env {
        DBLP_RECORDS_LENGTH: lines[0].split('=').collect::<Vec<&str>>()[1].parse().unwrap(),
        SURNAMES_LENGTH: lines[1].split('=').collect::<Vec<&str>>()[1].parse().unwrap(),
    };
}

pub fn hash(str: &str, M: usize) -> usize {
    let mut h: usize = 0;
    for c in str.chars() {
        h = h + c as usize;
    }
    h % M
}
